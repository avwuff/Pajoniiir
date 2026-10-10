/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/* First HS OUT packet fault per boot. Published once under the HCD lock;
 * readers copy only after acquire observes the immutable completed record.
 * No reset API: a reconnect must not erase evidence. Cycles wrap modulo 2^32. */
typedef struct {
    uint32_t sequence, root, channel, urb, list, start, packets, interval;
    uint32_t from_isr, irq_entry_cycle, irq_locked_cycle, irq_entry_hfnum;
    uint32_t activate_before_cycle, activate_after_cycle;
    uint32_t activate_before_hfnum, activate_after_hfnum;
    /* Sampled before the next buffer is activated, not in the later parser. */
    uint32_t done_cycle, done_hfnum, done_hcchar, done_hctsiz, done_hcdma;
    uint32_t done_ptx, done_buffers, done_urbs, done_stop_idx, done_hcint;
} usb_isoc_activation_t;

typedef struct {
    usb_isoc_activation_t activation;
    uint32_t completion_cycle, completion_hfnum, hcchar, hctsiz, hcdma, hcint;
    uint32_t ptx_status, first_packet, buffer_pending, urb_pending;
    /* Complete 64-entry list, two raw 32-bit words per QTD, before parsing. */
    uint32_t qtd[128];
} usb_isoc_fault_t;

bool usb_isoc_fault_enabled(void);
bool usb_isoc_fault_snapshot(usb_isoc_fault_t *out);
