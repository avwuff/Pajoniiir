/*
 * SPDX-FileCopyrightText: 2015-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/* Pinned espressif/usb HCD cache ownership fixture; exact source functions. */
static inline void cache_sync_data_buffer(pipe_t *pipe, urb_t *urb, bool done)
{
    const bool is_in = pipe->ep_char.bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
    const bool is_ctrl = (pipe->ep_char.type == USB_DWC_XFER_TYPE_CTRL);
    if ((is_in == done) || is_ctrl) {
        uint32_t flags = (done) ? ESP_CACHE_MSYNC_FLAG_DIR_M2C : ESP_CACHE_MSYNC_FLAG_UNALIGNED;
        esp_err_t ret = esp_cache_msync(urb->transfer.data_buffer, urb->transfer.data_buffer_size, flags);
        assert(ret == ESP_OK);
        (void)ret;
    }
}

urb_t *hcd_urb_dequeue(hcd_pipe_handle_t pipe_hdl)
{
    pipe_t *pipe = (pipe_t *)pipe_hdl;
    urb_t *urb;

    HCD_ENTER_CRITICAL();
    if (pipe->num_urb_done > 0) {
        urb = TAILQ_FIRST(&pipe->done_urb_tailq);
        TAILQ_REMOVE(&pipe->done_urb_tailq, urb, tailq_entry);
        pipe->num_urb_done--;
        // Check the URB's reserved fields then reset them
        assert(urb->hcd_ptr == (void *)pipe && urb->hcd_var == URB_HCD_STATE_DONE);  // The URB's reserved field should have been set to this pipe
        urb->hcd_ptr = NULL;
        urb->hcd_var = URB_HCD_STATE_IDLE;
        if (pipe->cs_flags.has_urb
                && pipe->num_urb_pending == 0 && pipe->num_urb_done == 0
                && pipe->multi_buffer_control.buffer_num_to_exec == 0 && pipe->multi_buffer_control.buffer_num_to_parse == 0) {
            // This pipe has no more enqueued URBs. Move the pipe to the list of idle pipes
            TAILQ_REMOVE(&pipe->port->pipes_active_tailq, pipe, tailq_entry);
            TAILQ_INSERT_TAIL(&pipe->port->pipes_idle_tailq, pipe, tailq_entry);
            pipe->port->num_pipes_idle++;
            pipe->port->num_pipes_queued--;
            pipe->cs_flags.has_urb = 0;
        }
        // Sync user's data in memory to cache. For IN and CTRL transfers
        CACHE_SYNC_DATA_BUFFER_M2C(pipe, urb);
    } else {
        // No more URBs to dequeue from this pipe
        urb = NULL;
    }
    HCD_EXIT_CRITICAL();
    return urb;
}
