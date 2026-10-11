"""Generate the S2 performance candidate from the pinned factory map.

Retains the PC-captured navigation overrides; basic profile remains unchanged.
Unlabelled controls/custom djay modifier behavior are never guessed.
"""
import importlib.util
import json
from pathlib import Path
import plistlib
import sys

basic_path = Path(__file__).parents[1] / 'ni_traktor_s2_mk3_basic/convert_djay.py'
spec = importlib.util.spec_from_file_location('s2_basic_converter', basic_path)
basic = importlib.util.module_from_spec(spec)
spec.loader.exec_module(basic)


def convert(source):
    profile = basic.convert(source)  # SHA and VID/PID checks, captured addresses
    mapping = plistlib.loads(source)
    profile['name'] = 'NI Traktor Kontrol S2 MK3 - performance candidate'
    profile['capabilities'].update(jog_touch=True, usb_audio=True,
                                   channel_filter_requires_smart_cfx=False)
    profile['audio'] = {'enabled': True, 'preferred_sample_rate': 48000,
                        'master_channels': [1, 2], 'headphone_channels': [3, 4]}
    inputs, outputs = profile['inputs'], profile['outputs']

    def control(key):
        items = [c for c in mapping['controls'] if c.get('keyPath') == key
                 and c.get('condition') is None]
        assert len(items) == 1, (key, items)
        return items[0]

    def address(c):
        assert c['midiMessageType'] in (1, 3)
        return {'status': hex((0x90 if c['midiMessageType'] == 1 else 0xb0)
                              | c['midiChannel']), 'data1': c['midiData']}

    def absolute(key, event, replay=True):
        c = control(key)
        assert c['midiMessageType'] == 3
        inputs.append({'type': 'cc7_to14_inverted' if c.get('flipped') else 'cc7_to14',
                       'event': event, 'replay': replay, **address(c)})

    absolute('mixer.crossfade', 'mixer.crossfader')
    absolute('mixer.monitorMix', 'mixer.headphone_mix')
    for deck in (1, 2):
        for key, event in (('gain', 'trim'), ('highEQ', 'eq_high'),
                           ('midEQ', 'eq_mid'), ('lowEQ', 'eq_low'),
                           ('filter', 'filter')):
            absolute(f'turntable{deck}.{key}', f'mixer.ch{deck}_{event}')
        absolute(f'mixer.lineVolume{deck}', f'mixer.ch{deck}_volume')
        absolute(f'turntable{deck}.speed', f'deck{deck}.tempo', replay=False)
        for key, event in (('scratchingMode', 'jog_touch'),
                           ('scratchingMove', 'jog_scratch'),
                           ('jogSeekMove', 'jog_search')):
            c = control(f'turntable{deck}.{key}')
            if key == 'scratchingMode':
                kind = 'button'
            else:
                assert c['controlType'] == 'rotary-64' and not c.get('flipped')
                kind = 'encoder_rel64'
            inputs.append({'type': kind, 'event': f'deck{deck}.{event}', **address(c)})
        master = control(f'turntable{deck}.turntableIsSyncMaster')
        inputs.append({'type': 'ext_action', 'deck': deck, 'action': 'sync_master',
                       **address(master)})
        shifted_status = 0x92 if deck == 1 else 0x94
        # Source identifies shifted Browse/back and shifted rotation separately.
        inputs.extend([
            {'type': 'button', 'event': 'browser.shift_press',
             'status': hex(shifted_status), 'data1': 0x18},
            {'type': 'encoder_rel64', 'event': 'browser.shift_delta',
             'status': hex(shifted_status + 0x20), 'data1': 0x19},
            # Product action on the captured dedicated LOAD address/channel.
            {'type': 'button', 'event': f'browser.shift_load_deck{deck}',
             'status': hex(shifted_status), 'data1': 0x13},
        ])
        # Explicit hot-cue output records, never inferred from generic pad LEDs.
        records = [c for c in mapping['controls']
                   if c.get('condition') == f'modifier{deck} == 0'
                   and c.get('keyPath', '').startswith(f'turntable{deck}.cueOrJumpIfAlreadySet')]
        records.sort(key=lambda c: int(c['keyPath'].rsplit('Set', 1)[1]))
        assert len(records) == 8
        for pad, c in enumerate(records):
            outputs.append({'kind': 'note', 'led': 'hot_cue_pads[%d]' % pad,
                            'deck': deck - 1, **address(c),
                            'off': int(c['output']['midiMinValue']),
                            'on': int(c['output']['midiMaxValue']), 'blink': 127})
        meter = next(c for c in mapping['outputs']
                     if c.get('keyPath') == f'turntable{deck}.monoMeter')
        outputs.append({'kind': 'note_velocity', 'led': 'vu_meter',
                        'deck': deck - 1, **address(meter)})
    return profile


if __name__ == '__main__':
    destination = Path(__file__).with_name('profile.json')
    destination.write_text(json.dumps(convert(Path(sys.argv[1]).read_bytes()), indent=2)+'\n',
                           encoding='utf-8')
    print(destination)
