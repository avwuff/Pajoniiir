/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define USB_BUS_TRACE_ROOTS 2u
#define USB_BUS_TRACE_CAPACITY 8u
enum { USB_BUS_TRACE_CHANNEL_ERROR = 1u, USB_BUS_TRACE_PORT_EVENT = 2u,
       USB_BUS_TRACE_ISOC_LATE_START = 3u };
typedef struct {
    uint32_t time_ms, kind, intr, hprt, hcchar, hctsiz, event;
} usb_bus_trace_event_t;
typedef struct {
    uint32_t total, count;
    usb_bus_trace_event_t events[USB_BUS_TRACE_CAPACITY];
} usb_bus_trace_snapshot_t;

/* One producer per root: its USB ISR. No allocation, logging, or device I/O.
 * Readers make at most three attempts and report unavailable on contention. */
void usb_bus_trace_record(unsigned root, const usb_bus_trace_event_t *event);
bool usb_bus_trace_snapshot(unsigned root, usb_bus_trace_snapshot_t *out);
