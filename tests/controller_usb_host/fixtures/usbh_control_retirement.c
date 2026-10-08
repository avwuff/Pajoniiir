/*
 * SPDX-FileCopyrightText: 2015-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 * Pinned esp-usb USBH functions, retained for the dependency race regression.
 */
static inline void handle_ep0_dequeue(device_t *dev_obj)
{
    // Empty URBs from EP0's pipe and call the control transfer callback
    ESP_LOGD(USBH_TAG, "Default pipe device %d", dev_obj->constant.address);
    int num_urbs = 0;
    urb_t *urb = hcd_urb_dequeue(dev_obj->constant.default_pipe);
    while (urb != NULL) {
        num_urbs++;
        usbh_event_data_t event_data = {
            .event = USBH_EVENT_CTRL_XFER,
            .ctrl_xfer_data = {
                .dev_hdl = (usb_device_handle_t)dev_obj,
                .urb = urb,
            },
        };
        p_usbh_obj->constant.event_cb(&event_data, p_usbh_obj->constant.event_cb_arg);
        urb = hcd_urb_dequeue(dev_obj->constant.default_pipe);
    }
    USBH_ENTER_CRITICAL();
    dev_obj->dynamic.num_ctrl_xfers_inflight -= num_urbs;
    USBH_EXIT_CRITICAL();
}

/* CLOSE */

esp_err_t usbh_dev_close(usb_device_handle_t dev_hdl)
{
    USBH_CHECK(dev_hdl != NULL, ESP_ERR_INVALID_ARG);
    device_t *dev_obj = (device_t *)dev_hdl;

    USBH_ENTER_CRITICAL();
    // Device should never be closed while its enum_lock is
    USBH_CHECK_FROM_CRIT(!dev_obj->dynamic.flags.enum_lock, ESP_ERR_NOT_ALLOWED);
    dev_obj->dynamic.open_count--;
    bool call_proc_req_cb = false;
    if (dev_obj->dynamic.open_count == 0) {
        // Sanity check.
        assert(dev_obj->dynamic.num_ctrl_xfers_inflight == 0);  // There cannot be any control transfer in-flight
        if (dev_obj->dynamic.flags.is_gone || dev_obj->dynamic.flags.waiting_free) {
            // Device is already gone or is awaiting to be freed. Trigger the USBH process to free the device
            call_proc_req_cb = _dev_set_actions(dev_obj, DEV_ACTION_FREE);
        }
        // Else, there's nothing to do. Leave the device allocated
    }
    USBH_EXIT_CRITICAL();

    if (call_proc_req_cb) {
        p_usbh_obj->constant.proc_req_cb(USB_PROC_REQ_SOURCE_USBH, false, p_usbh_obj->constant.proc_req_cb_arg);
    }

    return ESP_OK;
}
