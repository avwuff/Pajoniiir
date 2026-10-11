"""Read-only Windows WinMM short-message capture; no MIDI output or SysEx.

No dependencies beyond Python. Device IDs are WinMM indexes, not USB IDs.
Capture one named physical control per file to retain operator attribution.
"""
import argparse
import ctypes
from datetime import datetime, timezone
import json
from pathlib import Path
import queue
import sys
import time

DATA, ERROR, MORE = 0x3C3, 0x3C5, 0x3CC


def decode_short(packed):
    status = packed & 255
    if not 0x80 <= status <= 0xFF:
        raise ValueError("Invalid MIDI status")
    kind = status & 0xF0
    length = (2 if kind in (0xC0, 0xD0) else 3) if status < 0xF0 else {
        0xF1: 2, 0xF2: 3, 0xF3: 2,
        0xF6: 1, 0xF8: 1, 0xFA: 1, 0xFB: 1, 0xFC: 1, 0xFE: 1, 0xFF: 1,
    }.get(status)
    if length is None:
        raise ValueError("Unsupported system status (SysEx is not captured)")
    values = [(packed >> (8 * i)) & 255 for i in range(length)]
    if any(v > 127 for v in values[1:]):
        raise ValueError("Invalid MIDI data byte")
    return {"bytes": values, "status": f"0x{status:02X}",
            "channel": (status & 15) + 1 if status < 0xF0 else None,
            "data1": values[1] if length > 1 else None,
            "data2": values[2] if length > 2 else None}


class WinMM:
    def __init__(self):
        if sys.platform != "win32":
            raise RuntimeError("Live capture requires Windows; analyze works anywhere")
        from ctypes import wintypes as w
        self.w = w
        self.lib = ctypes.WinDLL("winmm")
        self.callback_type = ctypes.WINFUNCTYPE(
            None, w.HANDLE, w.UINT, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t)

        class Caps(ctypes.Structure):
            _fields_ = [("mid", w.WORD), ("pid", w.WORD), ("version", w.DWORD),
                        ("name", w.WCHAR * 32), ("support", w.DWORD)]
        self.caps_type = Caps
        signatures = {
            "midiInGetNumDevs": [],
            "midiInGetDevCapsW": [ctypes.c_size_t, ctypes.POINTER(Caps), w.UINT],
            "midiInOpen": [ctypes.POINTER(w.HANDLE), w.UINT, self.callback_type,
                           ctypes.c_size_t, w.DWORD],
            "midiInStart": [w.HANDLE], "midiInStop": [w.HANDLE],
            "midiInReset": [w.HANDLE], "midiInClose": [w.HANDLE],
        }
        for name, args in signatures.items():
            function = getattr(self.lib, name)
            function.argtypes, function.restype = args, w.UINT

    def check(self, name, *args):
        result = getattr(self.lib, name)(*args)
        if result:
            raise RuntimeError(f"{name} failed (WinMM error {result})")

    def devices(self):
        devices = []
        for index in range(self.lib.midiInGetNumDevs()):
            caps = self.caps_type()
            self.check("midiInGetDevCapsW", index, ctypes.byref(caps), ctypes.sizeof(caps))
            devices.append({"winmm_id": index, "name": caps.name,
                            "driver_manufacturer_id": caps.mid, "driver_product_id": caps.pid})
        return devices


def select_device(devices, device_id, name):
    matches = [d for d in devices if d["winmm_id"] == device_id] if device_id is not None else [
        d for d in devices if name.casefold() in d["name"].casefold()]
    if len(matches) != 1:
        raise ValueError(f"Select exactly one MIDI input; matched {len(matches)}")
    return matches[0]


def capture(args):
    api = WinMM()
    device = select_device(api.devices(), args.device_id, args.name)
    events = queue.Queue(maxsize=4096)
    counts = {"queue_dropped": 0, "driver_pressure": 0, "driver_errors": 0,
              "invalid_messages": 0, "messages": 0}

    def callback(handle, message, instance, packed, stamp):
        # No WinMM calls, console output or disk writes in the driver callback.
        if message in (DATA, MORE, ERROR):
            if message == MORE:
                counts["driver_pressure"] += 1
            if message == ERROR:
                counts["driver_errors"] += 1
            try:
                events.put_nowait((message, int(packed), int(stamp)))
            except queue.Full:
                counts["queue_dropped"] += 1

    # Keep this Python callback alive until midiInClose has completed.
    callback_ref = api.callback_type(callback)
    handle = api.w.HANDLE()
    opened = False
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("x", encoding="utf-8") as output:
        def emit(record):
            output.write(json.dumps(record) + "\n")
            output.flush()

        emit({"type": "metadata", "schema": "winmm-short-capture-v1",
              "utc": datetime.now(timezone.utc).isoformat(), "device": device,
              "label": args.label, "requested_seconds": args.seconds,
              "sysex_captured": False, "midi_output_sent": False})
        failure = None
        try:
            api.check("midiInOpen", ctypes.byref(handle), device["winmm_id"],
                      callback_ref, 0, 0x30000 | 0x20)
            opened = True
            api.check("midiInStart", handle)
            deadline = time.monotonic() + args.seconds
            print(f"Recording {device['name']}: {args.label} ({args.seconds:g}s)", flush=True)
            while time.monotonic() < deadline:
                try:
                    event = events.get(timeout=min(0.1, max(0.001, deadline - time.monotonic())))
                except queue.Empty:
                    continue
                write_event(event, emit, counts)
        except KeyboardInterrupt:
            failure = "operator_interrupted"
        except Exception as exc:
            failure = str(exc)
        finally:
            if opened:
                for operation in ("midiInStop", "midiInReset", "midiInClose"):
                    try:
                        api.check(operation, handle)
                    except RuntimeError as exc:
                        failure = str(exc)
                while not events.empty():
                    write_event(events.get_nowait(), emit, counts)
            complete = not failure and not any(counts[k] for k in (
                "queue_dropped", "driver_pressure", "driver_errors", "invalid_messages"))
            emit({"type": "summary", **counts, "complete": complete, "failure": failure})
    print(json.dumps({"path": str(args.out), **counts, "complete": complete}), flush=True)
    return 0 if complete and counts["messages"] else 2


def write_event(event, emit, counts):
    message, packed, stamp = event
    try:
        decoded = decode_short(packed)
    except ValueError as exc:
        counts["invalid_messages"] += 1
        emit({"type": "invalid", "driver_ms": stamp, "packed": packed, "error": str(exc)})
        return
    counts["messages"] += 1
    emit({"type": "midi", "driver_ms": stamp, "driver_event": message,
          "driver_error": message == ERROR, **decoded})


def analyze(path):
    addresses, summary, metadata = {}, None, None
    for line in path.read_text(encoding="utf-8").splitlines():
        record = json.loads(line)
        if record["type"] == "metadata":
            metadata = record
        elif record["type"] == "summary":
            summary = record
        elif record["type"] == "midi":
            key = (record["status"], record["data1"])
            entry = addresses.setdefault(key, {"status": key[0], "data1": key[1],
                                               "count": 0, "values": set()})
            entry["count"] += 1
            if record["data2"] is not None:
                entry["values"].add(record["data2"])
    entries = [{**e, "values": sorted(e["values"])} for e in addresses.values()]
    return {"metadata": metadata, "summary": summary, "addresses": entries,
            "usable_capture": bool(metadata and summary and summary.get("complete") and entries),
            "physical_semantics_confirmed": False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("list")
    record = commands.add_parser("record")
    source = record.add_mutually_exclusive_group(required=True)
    source.add_argument("--device-id", type=int)
    source.add_argument("--name", help="Unique case-insensitive device name substring")
    record.add_argument("--label", required=True, help="Physical control and movement order")
    record.add_argument("--seconds", type=float, default=15)
    record.add_argument("--out", required=True, type=Path)
    report = commands.add_parser("analyze")
    report.add_argument("path", type=Path)
    args = parser.parse_args()
    try:
        if args.command == "list":
            print(json.dumps(WinMM().devices(), indent=2))
        elif args.command == "analyze":
            print(json.dumps(analyze(args.path), indent=2))
        else:
            if not 0 < args.seconds <= 3600:
                parser.error("--seconds must be > 0 and <= 3600")
            return capture(args)
    except (OSError, RuntimeError, ValueError) as exc:
        parser.exit(1, f"{exc}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
