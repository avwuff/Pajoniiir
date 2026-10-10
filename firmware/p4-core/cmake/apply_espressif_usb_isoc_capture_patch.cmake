# Capture the first HS OUT descriptor failure without changing scheduling.
# Transform pinned HCD; final idle-recovery transform preserves these hooks.
set(_capture_includes_old "#include \"esp_idf_version.h\"")
set(_capture_includes_new [=[
#include "esp_idf_version.h"
#include "usb_isoc_fault.h"
#include "esp_cpu.h"
]=])
set(_capture_buffer_old "    urb_t *urb;\n    union {")
set(_capture_buffer_new [=[
    urb_t *urb;
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    usb_isoc_activation_t capture;
#endif
    union {
]=])
set(_capture_port_old "    void *frame_list;\n    // Pipes routed through this port")
set(_capture_port_new [=[
    void *frame_list;
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    uint32_t capture_irq_cycle, capture_lock_cycle, capture_irq_hfnum;
#endif
    // Pipes routed through this port
]=])
set(_capture_pipe_old "    usb_dwc_hal_chan_t *chan_obj;")
set(_capture_pipe_new [=[
    usb_dwc_hal_chan_t *chan_obj;
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    uint32_t capture_chan_intr;
#endif
]=])
set(_capture_decode_old "    usb_dwc_hal_chan_event_t chan_event = usb_dwc_hal_chan_decode_intr(chan_obj);")
set(_capture_decode_new [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    pipe->capture_chan_intr = chan_obj->regs->hcint_reg.val;
#endif
    usb_dwc_hal_chan_event_t chan_event = usb_dwc_hal_chan_decode_intr(chan_obj);
]=])
set(_capture_irq_old [=[
    HCD_ENTER_CRITICAL_ISR();
    usb_dwc_hal_port_event_t hal_port_evt = usb_dwc_hal_decode_intr(port->hal);
]=])
set(_capture_irq_new [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    const uint32_t capture_entry = esp_cpu_get_cycle_count();
    const uint32_t capture_frame = port->hal->dev->hfnum_reg.val;
#endif
    HCD_ENTER_CRITICAL_ISR();
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    port->capture_lock_cycle = esp_cpu_get_cycle_count();
    port->capture_irq_cycle = capture_entry;
    port->capture_irq_hfnum = capture_frame;
#endif
    usb_dwc_hal_port_event_t hal_port_evt = usb_dwc_hal_decode_intr(port->hal);
]=])
set(_capture_exec_old [=[
    usb_dwc_hal_chan_activate(pipe->chan_obj, buffer_to_exec->xfer_desc_list, desc_list_len, start_idx);
]=])
set(_capture_exec_new [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    const bool capture_out = pipe->port->hal->dev == USB_DWC_LL_GET_HW(0) &&
        pipe->ep_char.type == USB_DWC_XFER_TYPE_ISOCHRONOUS &&
        !(pipe->ep_char.bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) &&
        __atomic_load_n(&s_capture_state, __ATOMIC_RELAXED) == 0;
    if (capture_out) {
        usb_isoc_activation_t *c = &buffer_to_exec->capture;
        *c = (usb_isoc_activation_t){
            .sequence = ++s_capture_sequence, .root = 0,
            .channel = pipe->chan_obj->flags.chan_idx,
            .urb = (uint32_t)(uintptr_t)buffer_to_exec->urb,
            .list = (uint32_t)(uintptr_t)buffer_to_exec->xfer_desc_list,
            .start = start_idx, .packets = buffer_to_exec->urb->transfer.num_isoc_packets,
            .interval = buffer_to_exec->flags.isoc.interval,
            .from_isr = xPortInIsrContext(),
        };
        if (c->from_isr) {
            c->irq_entry_cycle = pipe->port->capture_irq_cycle;
            c->irq_locked_cycle = pipe->port->capture_lock_cycle;
            c->irq_entry_hfnum = pipe->port->capture_irq_hfnum;
        }
        c->activate_before_cycle = esp_cpu_get_cycle_count();
        c->activate_before_hfnum = pipe->port->hal->dev->hfnum_reg.val;
    }
#endif
    usb_dwc_hal_chan_activate(pipe->chan_obj, buffer_to_exec->xfer_desc_list, desc_list_len, start_idx);
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    if (capture_out) {
        buffer_to_exec->capture.activate_after_hfnum = pipe->port->hal->dev->hfnum_reg.val;
        buffer_to_exec->capture.activate_after_cycle = esp_cpu_get_cycle_count();
    }
#endif
]=])
set(_capture_parse_old "static inline void _buffer_parse_isoc(dma_buffer_block_t *buffer, bool is_in)\n{")
set(_capture_done_old "    buffer_done->status_flags.executing = 0;")
set(_capture_done_new [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    if (pipe->port->hal->dev == USB_DWC_LL_GET_HW(0) &&
        pipe->ep_char.type == USB_DWC_XFER_TYPE_ISOCHRONOUS &&
        !(pipe->ep_char.bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) &&
        __atomic_load_n(&s_capture_state, __ATOMIC_RELAXED) == 0) {
        usb_isoc_activation_t *c = &buffer_done->capture;
        c->done_cycle = esp_cpu_get_cycle_count();
        c->done_hfnum = pipe->port->hal->dev->hfnum_reg.val;
        c->done_hcchar = pipe->chan_obj->regs->hcchar_reg.val;
        c->done_hctsiz = pipe->chan_obj->regs->hctsiz_reg.val;
        c->done_hcdma = pipe->chan_obj->regs->hcdma_reg.val;
        c->done_ptx = pipe->port->hal->dev->hptxsts_reg.val;
        c->done_buffers = pipe->multi_buffer_control.buffer_num_to_exec;
        c->done_urbs = pipe->num_urb_pending;
        c->done_stop_idx = (uint32_t)stop_idx;
        c->done_hcint = pipe->capture_chan_intr;
    }
#endif
    buffer_done->status_flags.executing = 0;
]=])
set(_capture_parse_new [=[
static inline void _buffer_parse_isoc(pipe_t *pipe, dma_buffer_block_t *buffer, bool is_in)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    if (!is_in && pipe->port->hal->dev == USB_DWC_LL_GET_HW(0) &&
        __atomic_load_n(&s_capture_state, __ATOMIC_RELAXED) == 0) {
        unsigned idx = buffer->flags.isoc.start_idx;
        const usb_dwc_ll_dma_qtd_t *raw = buffer->xfer_desc_list;
        for (int p = 0; p < buffer->urb->transfer.num_isoc_packets; ++p) {
            /* Pinned LL maps active bit to NOT_EXECUTED, status bits 28:29
             * to packet/buffer error. Inspect after M2C, before destructive parse. */
            if (raw[idx].buffer_status_val & 0xb0000000u) {
                __atomic_store_n(&s_capture_state, 1u, __ATOMIC_RELAXED);
                usb_isoc_fault_t *f = &s_capture_fault;
                f->activation = buffer->capture;
                f->completion_cycle = buffer->capture.done_cycle;
                f->completion_hfnum = buffer->capture.done_hfnum;
                f->hcchar = buffer->capture.done_hcchar;
                f->hctsiz = buffer->capture.done_hctsiz;
                f->hcdma = buffer->capture.done_hcdma;
                f->hcint = buffer->capture.done_hcint;
                f->ptx_status = buffer->capture.done_ptx;
                f->first_packet = (uint32_t)p;
                f->buffer_pending = buffer->capture.done_buffers;
                f->urb_pending = buffer->capture.done_urbs;
                memcpy(f->qtd, raw, sizeof(f->qtd));
                __atomic_store_n(&s_capture_state, 2u, __ATOMIC_RELEASE);
                break;
            }
            idx = (idx + buffer->flags.isoc.interval) % XFER_LIST_LEN_ISOC;
        }
    }
#else
    (void)pipe;
#endif
]=])
set(_capture_state_old "// ------------------------------------------------- Forward Declare ---------------------------------------------------")
set(_capture_state_new [=[
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
static uint32_t s_capture_state, s_capture_sequence;
static usb_isoc_fault_t s_capture_fault;
#endif
bool usb_isoc_fault_enabled(void)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    return true;
#else
    return false;
#endif
}
bool usb_isoc_fault_snapshot(usb_isoc_fault_t *out)
{
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    if (!out || __atomic_load_n(&s_capture_state, __ATOMIC_ACQUIRE) != 2u) return false;
    *out = s_capture_fault; /* Immutable after release publication. */
    return true;
#else
    (void)out;
    return false;
#endif
}
// ------------------------------------------------- Forward Declare ---------------------------------------------------
]=])
set(_capture_call_old "_buffer_parse_isoc(buffer_to_parse, is_in);")
set(_capture_call_new "_buffer_parse_isoc(pipe, buffer_to_parse, is_in);")
foreach(_capture_part IN ITEMS includes buffer port pipe state irq decode exec done parse call)
    string(FIND "${_pajoniiir_hcd_contents}" "${_capture_${_capture_part}_old}" _capture_at)
    if(_capture_at EQUAL -1)
        message(FATAL_ERROR "Pinned HCD first ISO fault ${_capture_part} anchor changed")
    endif()
    string(REPLACE "${_capture_${_capture_part}_old}" "${_capture_${_capture_part}_new}"
        _pajoniiir_hcd_contents "${_pajoniiir_hcd_contents}")
endforeach()
