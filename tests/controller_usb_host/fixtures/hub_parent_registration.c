/* SPDX-License-Identifier: Apache-2.0. Pinned esp-usb cc65dc268f9f, src/hub.c. */
static esp_err_t dev_tree_node_new(ext_hub_handle_t parent, uint8_t port_num, usb_speed_t speed)
{
    esp_err_t ret;

    // Get root port handle of the new device
    hcd_port_handle_t root_port_hdl = NULL;
    if (parent == NULL) {
        assert(port_num < HCD_NUM_PORTS);
        root_port_hdl = p_hub_driver_obj->root_hub_ports[port_num].constant.hdl;
    } else {
#ifdef ENABLE_USB_HUBS
        root_port_hdl = ext_hub_get_root_port(parent);
#else
        abort(); // External hub support is not enabled. parent must be NULL for root port devices
#endif // ENABLE_USB_HUBS
    }
    HUB_DRIVER_CHECK(root_port_hdl != NULL, ESP_ERR_INVALID_ARG);

    // Allocate memory for a new device tree node
    dev_tree_node_t *dev_tree_node = heap_caps_calloc(1, sizeof(dev_tree_node_t), MALLOC_CAP_DEFAULT);
    if (dev_tree_node == NULL) {
        return ESP_ERR_NO_MEM;
    }
    // Assign initial UID based on the current number of registered devices
    int device_num = 0;
    ESP_ERROR_CHECK(usbh_devs_num(&device_num));
    dev_tree_node->uid = device_num + 1;
    // Ensure the UID is unique
    while (usbh_devs_is_uid_in_use(dev_tree_node->uid)) {
        dev_tree_node->uid++;
        assert(dev_tree_node->uid != 0); // No overflow possible
    }

    dev_tree_node->parent = parent;
    dev_tree_node->port_num = port_num;
    dev_tree_node->root_port_hdl = root_port_hdl;

    // Initialize and register a new USBH Device with the assigned UID
    usbh_dev_params_t params = {
        .uid = dev_tree_node->uid,
        .speed = speed,
        .root_port_hdl = root_port_hdl,
        // TODO: IDF-10023 Move parent-child tree management responsibility to Hub Driver
        .parent_dev_hdl = NULL,
        .parent_port_num = port_num,
    };

    ret = usbh_devs_add(&params);
    if (ret != ESP_OK) {
        // Device registration may fail if there are no available HCD channels.
        goto fail;
    }

    TAILQ_INSERT_TAIL(&p_hub_driver_obj->single_thread.dev_nodes_tailq, dev_tree_node, tailq_entry);

    ESP_LOGD(HUB_DRIVER_TAG, "Device tree node (port %d, uid %d): new", dev_tree_node->port_num, dev_tree_node->uid);

    hub_event_data_t event_data = {
        .event = HUB_EVENT_CONNECTED,
        .connected = {
            .uid = dev_tree_node->uid,
        },
    };
    p_hub_driver_obj->constant.event_cb(&event_data, p_hub_driver_obj->constant.event_cb_arg);

    return ret;

fail:
    heap_caps_free(dev_tree_node);
    return ret;
}
