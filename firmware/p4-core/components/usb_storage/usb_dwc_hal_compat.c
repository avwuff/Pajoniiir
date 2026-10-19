/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project-local ESP-IDF USB DWC channel-interrupt decoder.
 *
 * Why this exists
 * ---------------
 * usb_dwc_hal_chan_decode_intr() asserts that every channel error also raised
 * CHHLTD ("an error should have halted the channel"). The HAL's own contract
 * contradicts that for one case, in its header comment:
 *
 *     "When USB_DWC_LL_INTR_CHAN_BNAINTR occurs,
 *      USB_DWC_LL_INTR_CHAN_CHHLTD will NOT."
 *
 * and BNAINTR is nevertheless a member of CHAN_INTRS_ERROR_MSK. A BNA therefore
 * enters the error branch and trips the assertion. With
 * CONFIG_HAL_DEFAULT_ASSERTION_LEVEL=2 that is a panic and a reboot.
 *
 * This is not theoretical on this board. A 30-second capture on the P4 hit it
 * (docs/DEVELOPMENT_PLAN.md), and
 * docs/ARCHIVE_BENCH_NOTES_LEGACY_DUAL_PROCESSOR.md records the trigger
 * precisely: streaming from USB during playback. The original mitigation was to
 * preload each track into PSRAM so playback never touched USB. That mitigation
 * is gone - the bounded compressed cache streams from USB by design - so this
 * decoder is now the only thing standing between a BNA and a reboot mid-set.
 *
 * Why a full reimplementation rather than a delegating wrapper
 * -----------------------------------------------------------
 * The first statement of the real function reads *and clears* the interrupt
 * register. Peeking at the interrupts before delegating to __real_ would leave
 * it nothing to decode, so the logic has to be reproduced here. It is a mirror
 * of the ESP-IDF 6.0.2 implementation with exactly one behavioural difference:
 * BNA without CHHLTD is routed into the HCD's existing recoverable pipe-error
 * path instead of aborting. Every other missing-halt case still aborts, because
 * that genuinely does violate the invariant.
 *
 * Keeping it honest across IDF upgrades
 * ------------------------------------
 * A mirrored implementation silently rots when the original changes. The
 * version assertion below fails the build on any ESP-IDF other than the one
 * this was checked against, so an upgrade forces someone to re-read the
 * upstream function rather than discover the drift on hardware.
 */

#include <stdlib.h>

#include "esp_idf_version.h"
#include "hal/usb_dwc_hal.h"
#include "hal/usb_dwc_ll.h"
#include "esp_timer.h"
#include "usb_bus_trace.h"
#include "sdkconfig.h"
#include "esp_cache.h"
#include "hal/cache_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t __real_esp_cache_msync(void *address, size_t bytes, int flags);
#define P4_CACHE_SYNC_CHUNK_BYTES 1024u
esp_err_t __wrap_esp_cache_msync(void *address, size_t bytes, int flags)
{
#if CONFIG_ESP_MM_CACHE_MSYNC_C2M_CHUNKED_OPS
#error "P4 bounded writeback requires the global C2M mutex option disabled"
#endif
    /* Display/PPA and USB cache maintenance share IDF's interrupt-masking
     * spinlock. Ordinary FLX4/PCM5102 builds need the same task-context bound
     * as HS audio: selecting a controller must not enable long cache critical
     * sections. This bounds bytes, not elapsed time; exact-image physical
     * timing qualification remains required for every topology.
     * Bound task-context data sync without IDF's whole-operation mutex, which caused
     * audio/display contention in the previous diagnostic. DMA ownership and
     * publication remain with each caller until the complete sync returns. */
    uint32_t level, cache_id;
    uintptr_t end;
    if (address && bytes > P4_CACHE_SYNC_CHUNK_BYTES && xPortCanYield() &&
        !(flags & ESP_CACHE_MSYNC_FLAG_TYPE_INST) &&
        !((flags & ESP_CACHE_MSYNC_FLAG_DIR_M2C) &&
          (flags & (ESP_CACHE_MSYNC_FLAG_UNALIGNED | ESP_CACHE_MSYNC_FLAG_DIR_C2M))) &&
        !__builtin_add_overflow((uintptr_t)address, bytes, &end) &&
        cache_hal_vaddr_to_cache_level_id((uint32_t)(uintptr_t)address, bytes, &level, &cache_id)) {
        const size_t line = cache_hal_get_cache_line_size(level, CACHE_TYPE_DATA);
        if (line && (P4_CACHE_SYNC_CHUNK_BYTES % line) == 0 &&
            ((flags & ESP_CACHE_MSYNC_FLAG_UNALIGNED) ||
             (((uintptr_t)address % line) == 0 && (bytes % line) == 0))) {
            uint8_t *cursor = address;
            while (bytes) {
                const size_t count = bytes > P4_CACHE_SYNC_CHUNK_BYTES ? P4_CACHE_SYNC_CHUNK_BYTES : bytes;
                const esp_err_t rc = __real_esp_cache_msync(cursor, count, flags);
                if (rc != ESP_OK) return rc;
                cursor += count;
                bytes -= count;
            }
            return ESP_OK;
        }
    }
    return __real_esp_cache_msync(address, bytes, flags);
}

void __real_usb_dwc_hal_chan_activate(usb_dwc_hal_chan_t *chan, void *list,
                                    int length, int start);
void __wrap_usb_dwc_hal_chan_activate(usb_dwc_hal_chan_t *chan, void *list,
                                    int length, int start)
{
#if CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
    /* Capture after activation, publish only after the channel is enabled.
     * HFNUM: low 16 bits are the microframe number, upper bits time remaining.
     * Kind 3 event=start index, intr=HFNUM. This is evidence, not recovery.
     * HCD serializes activation and interrupt decoding under its own lock. */
    usb_dwc_dev_t *dev = USB_DWC_LL_GET_HW(0);
    const bool hs_out_isoc = chan->regs == &dev->host_chans[chan->flags.chan_idx] &&
        length == 64 && ((chan->regs->hcchar_reg.val >> 18) & 3u) == 1u &&
        !(chan->regs->hcchar_reg.val & (1u << 15));
#endif
    __real_usb_dwc_hal_chan_activate(chan, list, length, start);
#if CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
    /* A microframe can advance while HAL programs the channel. Pre-activation
     * HFNUM alone could classify an already-late enable as one frame early. */
    const uint32_t frame = hs_out_isoc ? dev->hfnum_reg.val : 0u;
    const uint32_t lead = ((uint32_t)start - (frame & 63u)) & 63u;
    if (hs_out_isoc && (lead == 0u || lead > 32u)) {
        const usb_bus_trace_event_t event = {
            .time_ms = (uint32_t)(esp_timer_get_time() / 1000),
            .kind = USB_BUS_TRACE_ISOC_LATE_START, .intr = frame,
            .hprt = dev->hprt_reg.val, .hcchar = chan->regs->hcchar_reg.val,
            .hctsiz = chan->regs->hctsiz_reg.val, .event = (uint32_t)start,
        };
        usb_bus_trace_record(0u, &event);
    }
#endif
}

/* Capture only port transitions and channel errors. Register reads have no
 * side effects; the original decoder remains the sole interrupt acknowledger. */
usb_dwc_hal_port_event_t __real_usb_dwc_hal_decode_intr(usb_dwc_hal_context_t *hal);
usb_dwc_hal_port_event_t __wrap_usb_dwc_hal_decode_intr(usb_dwc_hal_context_t *hal)
{
#if CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
    const uint32_t intr = hal->dev->gintsts_reg.val;
    const uint32_t hprt = hal->dev->hprt_reg.val;
    const usb_dwc_hal_port_event_t event = __real_usb_dwc_hal_decode_intr(hal);
    if (event != USB_DWC_HAL_PORT_EVENT_NONE && event != USB_DWC_HAL_PORT_EVENT_CHAN) {
        const usb_bus_trace_event_t record = {
            .time_ms = (uint32_t)(esp_timer_get_time() / 1000),
            .kind = USB_BUS_TRACE_PORT_EVENT, .intr = intr, .hprt = hprt,
            .event = (uint32_t)event,
        };
        for (unsigned root = 0; root < USB_BUS_TRACE_ROOTS; ++root) {
            if (hal->dev == USB_DWC_LL_GET_HW(root)) {
                usb_bus_trace_record(root, &record);
                break;
            }
        }
    }
    return event;
#else
    return __real_usb_dwc_hal_decode_intr(hal);
#endif
}

#if CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
static void record_channel_error(usb_dwc_hal_chan_t *chan, uint32_t intr)
{
    for (unsigned root = 0; root < USB_BUS_TRACE_ROOTS; ++root) {
        usb_dwc_dev_t *dev = USB_DWC_LL_GET_HW(root);
        if (chan->regs != &dev->host_chans[chan->flags.chan_idx]) continue;
        const usb_bus_trace_event_t record = {
            .time_ms = (uint32_t)(esp_timer_get_time() / 1000),
            .kind = USB_BUS_TRACE_CHANNEL_ERROR, .intr = intr,
            .hprt = dev->hprt_reg.val, .hcchar = chan->regs->hcchar_reg.val,
            .hctsiz = chan->regs->hctsiz_reg.val, .event = (uint32_t)chan->error,
        };
        usb_bus_trace_record(root, &record);
        break;
    }
}
#endif

#if (ESP_IDF_VERSION_MAJOR != 6) || (ESP_IDF_VERSION_MINOR != 0)
#error "Re-read usb_dwc_hal_chan_decode_intr() in this ESP-IDF before building. \
This file mirrors the 6.0.x implementation and must be verified against any other."
#endif

#define DDJ_DWC_CHAN_ERROR_MASK (USB_DWC_LL_INTR_CHAN_STALL | \
                                 USB_DWC_LL_INTR_CHAN_BBLEER | \
                                 USB_DWC_LL_INTR_CHAN_BNAINTR | \
                                 USB_DWC_LL_INTR_CHAN_XCS_XACT_ERR)

/* Observability for the hardware acceptance run that is still open on this:
 * "reproduce sustained USB playback/storage activity and confirm BNA recovery".
 * Without a counter that check has nothing to look at - a silent recovery and a
 * BNA that never happened are indistinguishable. The ISR increments atomically
 * because the web diagnostics task reads this value concurrently. */
static uint32_t s_bna_recovered_count;

uint32_t usb_dwc_compat_bna_recovered_count(void)
{
    return __atomic_load_n(&s_bna_recovered_count, __ATOMIC_ACQUIRE);
}

usb_dwc_hal_chan_event_t __wrap_usb_dwc_hal_chan_decode_intr(
    usb_dwc_hal_chan_t *chan_obj)
{
    uint32_t chan_intrs =
        usb_dwc_ll_hcint_read_and_clear_intrs(chan_obj->regs);

    /* Order matters and mirrors upstream: errors > halt request > completed. */
    if (chan_intrs & DDJ_DWC_CHAN_ERROR_MASK) {
        const bool halted = (chan_intrs & USB_DWC_LL_INTR_CHAN_CHHLTD) != 0u;
        const bool bna = (chan_intrs & USB_DWC_LL_INTR_CHAN_BNAINTR) != 0u;
        if (!halted && !bna) {
            /* Upstream's invariant, kept: any error other than BNA really is
             * expected to have halted the channel, and a violation is a bug
             * worth failing loudly for. */
            abort();
        }
        if (!halted && bna) {
            (void)__atomic_add_fetch(&s_bna_recovered_count, 1u,
                                     __ATOMIC_RELAXED);
        }

        usb_dwc_hal_chan_error_t error;
        if (chan_intrs & USB_DWC_LL_INTR_CHAN_STALL) {
            error = USB_DWC_HAL_CHAN_ERROR_STALL;
        } else if (chan_intrs & USB_DWC_LL_INTR_CHAN_BBLEER) {
            error = USB_DWC_HAL_CHAN_ERROR_PKT_BBL;
        } else if (chan_intrs & USB_DWC_LL_INTR_CHAN_BNAINTR) {
            error = USB_DWC_HAL_CHAN_ERROR_BNA;
        } else {   /* USB_DWC_LL_INTR_CHAN_XCS_XACT_ERR */
            error = USB_DWC_HAL_CHAN_ERROR_XCS_XACT;
        }
        chan_obj->error = error;
#if CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE
        record_channel_error(chan_obj, chan_intrs);
#endif
        chan_obj->flags.active = 0;
        return USB_DWC_HAL_CHAN_EVENT_ERROR;
    }

    if (chan_intrs & USB_DWC_LL_INTR_CHAN_CHHLTD) {
        usb_dwc_hal_chan_event_t event;
        if (chan_obj->flags.halt_requested) {
            chan_obj->flags.halt_requested = 0;
            event = USB_DWC_HAL_CHAN_EVENT_HALT_REQ;
        } else {
            /* Halted by QTD HOC. */
            event = USB_DWC_HAL_CHAN_EVENT_CPLT;
        }
        chan_obj->flags.active = 0;
        return event;
    }

    if (chan_intrs & USB_DWC_LL_INTR_CHAN_XFERCOMPL) {
        /* Transfer complete without a halt: a short interrupt IN packet whose
         * QTD has no HOC bit. Halt manually so the next QTD does not run; the
         * resulting halt interrupt comes back round as the CPLT event. */
        usb_dwc_ll_hcchar_disable_chan(chan_obj->regs);
        return USB_DWC_HAL_CHAN_EVENT_NONE;
    }

    abort();
}
