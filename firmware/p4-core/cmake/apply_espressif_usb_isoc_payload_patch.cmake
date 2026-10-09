# Keep periodic DMA payloads off the contended external-memory path on the
# experimental M3 HS-audio target. Bulk/control retain the pinned policy.
set(_payload_source "${_pajoniiir_usb_dir}/src/usb_private.c")
file(READ "${_payload_source}" _payload_contents)
set(_payload_old [=[
urb_t *urb_alloc(size_t data_buffer_size, int num_isoc_packets)
{
]=])
set(_payload_new [=[
urb_t *urb_alloc(size_t data_buffer_size, int num_isoc_packets)
{
    uint32_t payload_caps = DATA_BUFFER_CAPS;
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
    /* No PSRAM fallback: a missed periodic DMA deadline loses audio.
     * The stream retains only idle URBs across reconnect and validates their
     * capacity. Control/bulk remain in PSRAM to preserve internal headroom. */
    if (num_isoc_packets > 0)
        payload_caps = MALLOC_CAP_DMA | MALLOC_CAP_CACHE_ALIGNED | MALLOC_CAP_INTERNAL;
#endif
]=])
foreach(_payload_anchor IN ITEMS "${_payload_old}" "esp_cache_get_alignment(DATA_BUFFER_CAPS, &cache_align);" "heap_caps_malloc(data_buffer_size, DATA_BUFFER_CAPS);")
    string(FIND "${_payload_contents}" "${_payload_anchor}" _payload_at)
    if(_payload_at EQUAL -1)
        message(FATAL_ERROR "Pinned USB payload allocation contract changed")
    endif()
endforeach()
string(REPLACE "${_payload_old}" "${_payload_new}" _payload_contents "${_payload_contents}")
string(REPLACE "esp_cache_get_alignment(DATA_BUFFER_CAPS, &cache_align);" "esp_cache_get_alignment(payload_caps, &cache_align);" _payload_contents "${_payload_contents}")
string(REPLACE "heap_caps_malloc(data_buffer_size, DATA_BUFFER_CAPS);" "heap_caps_malloc(data_buffer_size, payload_caps);" _payload_contents "${_payload_contents}")
set(_payload_generated "${_pajoniiir_generated_dir}/usb_private.c")
file(WRITE "${_payload_generated}" "${_payload_contents}")
set_source_files_properties("${_payload_generated}" PROPERTIES GENERATED TRUE)
set(_payload_sources)
set(_payload_found FALSE)
foreach(_src IN LISTS _pajoniiir_usb_replacement_sources)
    get_filename_component(_abs "${_src}" ABSOLUTE BASE_DIR "${_pajoniiir_usb_dir}")
    if(_abs STREQUAL _payload_source)
        list(APPEND _payload_sources "${_payload_generated}")
        set(_payload_found TRUE)
    else()
        list(APPEND _payload_sources "${_src}")
    endif()
endforeach()
if(NOT _payload_found)
    message(FATAL_ERROR "Pinned USB payload source not registered")
endif()
set(_pajoniiir_usb_replacement_sources "${_payload_sources}")
