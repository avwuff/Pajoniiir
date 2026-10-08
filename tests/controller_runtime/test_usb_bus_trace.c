/* SPDX-License-Identifier: Apache-2.0 */
#include "usb_bus_trace.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

static _Atomic bool done;
static usb_bus_trace_event_t record_for(uint32_t n)
{
    return (usb_bus_trace_event_t){n, 1, n ^ 0xfeedu, n + 1, n + 2, n + 3, n + 4};
}
static void check(const usb_bus_trace_snapshot_t *s)
{
    assert(s->count <= USB_BUS_TRACE_CAPACITY);
    for (uint32_t i = 0; i < s->count; ++i) {
        const usb_bus_trace_event_t *e = &s->events[i];
        assert(e->kind == 1 && e->intr == (e->time_ms ^ 0xfeedu));
        assert(e->hprt == e->time_ms + 1 && e->hcchar == e->time_ms + 2);
        assert(e->hctsiz == e->time_ms + 3 && e->event == e->time_ms + 4);
        if (i) assert(e->time_ms == s->events[i - 1].time_ms + 1);
    }
}
static void *producer(void *arg)
{
    (void)arg;
    for (uint32_t n = 11; n <= 100000; ++n) {
        usb_bus_trace_event_t e = record_for(n);
        usb_bus_trace_record(0, &e);
    }
    atomic_store(&done, true);
    return NULL;
}
int main(void)
{
    usb_bus_trace_snapshot_t s;
    assert(!usb_bus_trace_snapshot(2, &s));
    assert(!usb_bus_trace_snapshot(0, NULL));
    usb_bus_trace_record(0, NULL);
    assert(usb_bus_trace_snapshot(0, &s) && s.count == 0 && s.total == 0);
    for (uint32_t n = 1; n <= 10; ++n) {
        usb_bus_trace_event_t e = record_for(n);
        usb_bus_trace_record(0, &e);
    }
    assert(usb_bus_trace_snapshot(0, &s) && s.count == 8 && s.total == 10);
    assert(s.events[0].time_ms == 3 && s.events[7].time_ms == 10);
    check(&s);
    assert(usb_bus_trace_snapshot(1, &s) && s.total == 0);
    usb_bus_trace_event_t other = record_for(42);
    usb_bus_trace_record(1, &other);
    pthread_t thread;
    assert(pthread_create(&thread, NULL, producer, NULL) == 0);
    do {
        if (usb_bus_trace_snapshot(0, &s)) check(&s);
    } while (!atomic_load(&done));
    assert(pthread_join(thread, NULL) == 0);
    assert(usb_bus_trace_snapshot(0, &s) && s.total == 100000);
    check(&s);
    assert(s.events[7].time_ms == 100000);
    assert(usb_bus_trace_snapshot(1, &s) && s.total == 1 && s.events[0].time_ms == 42);
    puts("PASS USB bus trace: bounded history, root isolation, coherent concurrent snapshots");
    return 0;
}
