# Retire EP0 ownership before waking a client that can close the device.
# The pinned esp-usb publishes completions before decrementing the device's
# in-flight count. A higher-priority client can finish teardown in that gap.
# Keep upstream assertions; replace only this reviewed block, fail closed.
set(_p4_ctrl_upstream [=[
        num_urbs++;
        usbh_event_data_t event_data = {
]=])
set(_p4_ctrl_patched [=[
        // Ownership must be retired before a callback can wake the client.
        // Device freeing is deferred to usbh_process(), after this handler.
        USBH_ENTER_CRITICAL();
        assert(dev_obj->dynamic.num_ctrl_xfers_inflight > 0);
        dev_obj->dynamic.num_ctrl_xfers_inflight--;
        USBH_EXIT_CRITICAL();
        usbh_event_data_t event_data = {
]=])
set(_p4_ctrl_tail [=[
    USBH_ENTER_CRITICAL();
    dev_obj->dynamic.num_ctrl_xfers_inflight -= num_urbs;
    USBH_EXIT_CRITICAL();
]=])
idf_component_get_property(_p4_ctrl_dir usb COMPONENT_DIR)
idf_component_get_property(_p4_ctrl_lib usb COMPONENT_LIB)
set(_p4_ctrl_source "${_p4_ctrl_dir}/src/usbh.c")
file(READ "${_p4_ctrl_source}" _p4_ctrl_contents)
foreach(_p4_ctrl_block IN ITEMS "    int num_urbs = 0;\n" "${_p4_ctrl_upstream}" "${_p4_ctrl_tail}")
    string(FIND "${_p4_ctrl_contents}" "${_p4_ctrl_block}" _p4_ctrl_at)
    if(_p4_ctrl_at EQUAL -1)
        message(FATAL_ERROR "esp-usb EP0 retirement block changed; review the control ownership patch")
    endif()
endforeach()
string(REPLACE "    int num_urbs = 0;\n" "" _p4_ctrl_contents "${_p4_ctrl_contents}")
string(REPLACE "${_p4_ctrl_upstream}" "${_p4_ctrl_patched}" _p4_ctrl_contents "${_p4_ctrl_contents}")
string(REPLACE "${_p4_ctrl_tail}" "" _p4_ctrl_contents "${_p4_ctrl_contents}")
set(_p4_ctrl_generated "${CMAKE_BINARY_DIR}/pajoniiir_usb_control/usbh.c")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pajoniiir_usb_control")
file(WRITE "${_p4_ctrl_generated}" "${_p4_ctrl_contents}")
set_source_files_properties("${_p4_ctrl_generated}" PROPERTIES GENERATED TRUE)
get_target_property(_p4_ctrl_sources "${_p4_ctrl_lib}" SOURCES)
set(_p4_ctrl_replacements)
set(_p4_ctrl_found FALSE)
foreach(_p4_ctrl_item IN LISTS _p4_ctrl_sources)
    get_filename_component(_p4_ctrl_abs "${_p4_ctrl_item}" ABSOLUTE BASE_DIR "${_p4_ctrl_dir}")
    if(_p4_ctrl_abs STREQUAL _p4_ctrl_source)
        list(APPEND _p4_ctrl_replacements "${_p4_ctrl_generated}")
        set(_p4_ctrl_found TRUE)
    else()
        list(APPEND _p4_ctrl_replacements "${_p4_ctrl_item}")
    endif()
endforeach()
if(NOT _p4_ctrl_found)
    message(FATAL_ERROR "Could not replace esp-usb usbh.c for EP0 ownership retirement")
endif()
set_property(TARGET "${_p4_ctrl_lib}" PROPERTY SOURCES "${_p4_ctrl_replacements}")
message(STATUS "Using generated Pajoniiir esp-usb control-retirement source")
