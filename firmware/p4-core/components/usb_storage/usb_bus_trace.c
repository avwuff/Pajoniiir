/* SPDX-License-Identifier: Apache-2.0 */
#include "usb_bus_trace.h"
#include <stdatomic.h>
#include <stddef.h>
#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#endif

#if !defined(ESP_PLATFORM) || CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
_Static_assert(ATOMIC_INT_LOCK_FREE == 2, "ISR trace requires lock-free 32-bit atomics");

typedef struct {
    _Atomic uint32_t time_ms, kind, intr, hprt, hcchar, hctsiz, event;
} atomic_event_t;
static struct {
    _Atomic uint32_t sequence, total, next, count;
    atomic_event_t events[USB_BUS_TRACE_CAPACITY];
} s_roots[USB_BUS_TRACE_ROOTS];

void usb_bus_trace_record(unsigned root, const usb_bus_trace_event_t *event)
{
    if (root >= USB_BUS_TRACE_ROOTS || !event) return;
    /* Sequentially consistent atomics include payload words: readers never
     * race on plain C objects, even when the ISR overwrites an old slot. */
    atomic_fetch_add(&s_roots[root].sequence, 1u);
    uint32_t next = atomic_load(&s_roots[root].next);
    atomic_event_t *slot = &s_roots[root].events[next];
#define STORE(field) atomic_store(&slot->field, event->field)
    STORE(time_ms); STORE(kind); STORE(intr); STORE(hprt);
    STORE(hcchar); STORE(hctsiz); STORE(event);
#undef STORE
    atomic_store(&s_roots[root].next, (next + 1u) % USB_BUS_TRACE_CAPACITY);
    uint32_t count = atomic_load(&s_roots[root].count);
    if (count < USB_BUS_TRACE_CAPACITY) atomic_store(&s_roots[root].count, count + 1u);
    atomic_fetch_add(&s_roots[root].total, 1u);
    atomic_fetch_add(&s_roots[root].sequence, 1u);
}

bool usb_bus_trace_snapshot(unsigned root, usb_bus_trace_snapshot_t *out)
{
    if (root >= USB_BUS_TRACE_ROOTS || !out) return false;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        uint32_t seq = atomic_load(&s_roots[root].sequence);
        if (seq & 1u) continue;
        out->total = atomic_load(&s_roots[root].total);
        out->count = atomic_load(&s_roots[root].count);
        uint32_t next = atomic_load(&s_roots[root].next);
        uint32_t first = (next + USB_BUS_TRACE_CAPACITY - out->count) % USB_BUS_TRACE_CAPACITY;
        for (uint32_t i = 0; i < out->count; ++i) {
            atomic_event_t *slot = &s_roots[root].events[(first + i) % USB_BUS_TRACE_CAPACITY];
            usb_bus_trace_event_t *event = &out->events[i];
#define LOAD(field) event->field = atomic_load(&slot->field)
            LOAD(time_ms); LOAD(kind); LOAD(intr); LOAD(hprt);
            LOAD(hcchar); LOAD(hctsiz); LOAD(event);
#undef LOAD
        }
        if (seq == atomic_load(&s_roots[root].sequence)) return true;
    }
    return false;
}
#else
void usb_bus_trace_record(unsigned root, const usb_bus_trace_event_t *event)
{
    (void)root; (void)event;
}
bool usb_bus_trace_snapshot(unsigned root, usb_bus_trace_snapshot_t *out)
{
    (void)root; (void)out;
    return false;
}
#endif
