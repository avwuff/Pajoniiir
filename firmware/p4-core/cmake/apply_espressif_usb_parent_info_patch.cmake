# The pinned hub owner supplies NULL for every device's parent (IDF-10023).
# Preserve the real parent handle at creation; never infer root from a hub port.
idf_component_get_property(_p4_parent_dir usb COMPONENT_DIR)
idf_component_get_property(_p4_parent_lib usb COMPONENT_LIB)
get_target_property(_p4_parent_sources "${_p4_parent_lib}" SOURCES)
set(_p4_parent_input "")
foreach(_p4_parent_source IN LISTS _p4_parent_sources)
    get_filename_component(_p4_parent_abs "${_p4_parent_source}" ABSOLUTE BASE_DIR "${_p4_parent_dir}")
    get_filename_component(_p4_parent_name "${_p4_parent_abs}" NAME)
    if(_p4_parent_name STREQUAL "hub.c")
        if(NOT _p4_parent_input STREQUAL "")
            message(FATAL_ERROR "Multiple hub sources selected before ancestry patch")
        endif()
        set(_p4_parent_input "${_p4_parent_abs}")
    endif()
endforeach()
if(_p4_parent_input STREQUAL "")
    message(FATAL_ERROR "Missing selected USB hub source for ancestry patch")
endif()
set(_p4_parent_old [=[
        // TODO: IDF-10023 Move parent-child tree management responsibility to Hub Driver
        .parent_dev_hdl = NULL,
]=])
set(_p4_parent_new [=[
        // Preserve ancestry for root-specific admission and recovery.
        .parent_dev_hdl = parent_dev_hdl,
]=])
# Compose with the earlier indexed-idle-recovery transform, never discard it.
file(READ "${_p4_parent_input}" _p4_parent_hub)
string(FIND "${_p4_parent_hub}" "${_p4_parent_old}" _p4_parent_at)
if(_p4_parent_at EQUAL -1)
    message(FATAL_ERROR "esp-usb parent metadata changed; review ancestry patch")
endif()
string(REPLACE "${_p4_parent_old}" "${_p4_parent_new}" _p4_parent_hub "${_p4_parent_hub}")
set(_p4_parent_anchor "    usbh_dev_params_t params = {\n")
set(_p4_parent_setup [=[
    usb_device_handle_t parent_dev_hdl = NULL;
#if ENABLE_USB_HUBS
    if (parent) {
        extern usb_device_handle_t pajoniiir_ext_hub_device_handle(ext_hub_handle_t);
        parent_dev_hdl = pajoniiir_ext_hub_device_handle(parent);
        if (!parent_dev_hdl) {
            heap_caps_free(dev_tree_node);
            return ESP_ERR_NOT_FOUND;
        }
    }
#endif
    usbh_dev_params_t params = {
]=])
string(FIND "${_p4_parent_hub}" "${_p4_parent_anchor}" _p4_parent_at)
if(_p4_parent_at EQUAL -1)
    message(FATAL_ERROR "esp-usb hub registration changed; review ancestry patch")
endif()
string(REPLACE "${_p4_parent_anchor}" "${_p4_parent_setup}" _p4_parent_hub "${_p4_parent_hub}")
file(READ "${_p4_parent_dir}/src/ext_hub.c" _p4_parent_ext)
string(FIND "${_p4_parent_ext}" "ext_hub_dev->constant.dev_hdl" _p4_parent_at)
if(_p4_parent_at EQUAL -1)
    message(FATAL_ERROR "esp-usb external hub ownership changed; review ancestry patch")
endif()
string(APPEND _p4_parent_ext [=[

/* Called only by the hub owner's enumeration path while the parent is live. */
usb_device_handle_t pajoniiir_ext_hub_device_handle(ext_hub_handle_t handle)
{
    if (!handle || !dev_is_in_list(handle)) return NULL;
    return ((ext_hub_dev_t *)handle)->constant.dev_hdl;
}
]=])
set(_p4_parent_generated "${CMAKE_BINARY_DIR}/pajoniiir_usb_parent")
file(MAKE_DIRECTORY "${_p4_parent_generated}")
file(WRITE "${_p4_parent_generated}/hub.c" "${_p4_parent_hub}")
file(WRITE "${_p4_parent_generated}/ext_hub.c" "${_p4_parent_ext}")
get_target_property(_p4_parent_sources "${_p4_parent_lib}" SOURCES)
set(_p4_parent_replaced)
set(_p4_parent_found FALSE)
foreach(_p4_parent_source IN LISTS _p4_parent_sources)
    get_filename_component(_p4_parent_abs "${_p4_parent_source}" ABSOLUTE BASE_DIR "${_p4_parent_dir}")
    if(_p4_parent_abs STREQUAL _p4_parent_input)
        list(APPEND _p4_parent_replaced "${_p4_parent_generated}/hub.c")
        set(_p4_parent_found TRUE)
    elseif(_p4_parent_abs STREQUAL "${_p4_parent_dir}/src/ext_hub.c")
        list(APPEND _p4_parent_replaced "${_p4_parent_generated}/ext_hub.c")
    else()
        list(APPEND _p4_parent_replaced "${_p4_parent_source}")
    endif()
endforeach()
if(NOT _p4_parent_found)
    message(FATAL_ERROR "Could not replace esp-usb hub source for root ancestry")
endif()
set_property(TARGET "${_p4_parent_lib}" PROPERTY SOURCES "${_p4_parent_replaced}")
message(STATUS "Using generated Pajoniiir USB parent metadata sources")
