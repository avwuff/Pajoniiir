# M3 microSD slot 0 keeps the legacy host alive when Hosted releases slot 1.
# Reuse that host on OFF->ON; never recreate or deinitialize the other owner.
set(_p4_sdio_old [=[
	// initialise SDMMC host
	res = sdmmc_host_init();
	if (res != ESP_OK) {
]=])
set(_p4_sdio_new [=[
	// initialise SDMMC host, preserving a mounted microSD owner on IDF 6.
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
	sdmmc_host_state_t shared_host_state = {0};
	res = sdmmc_host_get_state(&shared_host_state);
	if (res == ESP_OK && !shared_host_state.host_initialized) {
		res = sdmmc_host_init();
	}
#else
	res = sdmmc_host_init();
#endif
	if (res != ESP_OK) {
]=])
idf_component_get_property(_p4_sdio_dir espressif__esp_hosted COMPONENT_DIR)
idf_component_get_property(_p4_sdio_lib espressif__esp_hosted COMPONENT_LIB)
set(_p4_sdio_source "${_p4_sdio_dir}/host/port/esp/freertos/src/port_esp_hosted_host_sdio.c")
file(READ "${_p4_sdio_source}" _p4_sdio_contents)
string(FIND "${_p4_sdio_contents}" "${_p4_sdio_old}" _p4_sdio_at)
if(_p4_sdio_at EQUAL -1)
    message(FATAL_ERROR "Pinned Hosted SDMMC init changed; review shared-host patch")
endif()
string(REPLACE "${_p4_sdio_old}" "${_p4_sdio_new}" _p4_sdio_contents "${_p4_sdio_contents}")
set(_p4_sdio_generated "${CMAKE_BINARY_DIR}/pajoniiir_hosted_shared/port_esp_hosted_host_sdio.c")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pajoniiir_hosted_shared")
file(WRITE "${_p4_sdio_generated}" "${_p4_sdio_contents}")
set_source_files_properties("${_p4_sdio_generated}" PROPERTIES GENERATED TRUE)
get_target_property(_p4_sdio_sources "${_p4_sdio_lib}" SOURCES)
set(_p4_sdio_replacements)
set(_p4_sdio_found FALSE)
foreach(_p4_sdio_item IN LISTS _p4_sdio_sources)
    get_filename_component(_p4_sdio_abs "${_p4_sdio_item}" ABSOLUTE BASE_DIR "${_p4_sdio_dir}")
    if(_p4_sdio_abs STREQUAL _p4_sdio_source)
        list(APPEND _p4_sdio_replacements "${_p4_sdio_generated}")
        set(_p4_sdio_found TRUE)
    else()
        list(APPEND _p4_sdio_replacements "${_p4_sdio_item}")
    endif()
endforeach()
if(NOT _p4_sdio_found)
    message(FATAL_ERROR "Could not replace Hosted SDIO source for shared host lifecycle")
endif()
set_property(TARGET "${_p4_sdio_lib}" PROPERTY SOURCES "${_p4_sdio_replacements}")
message(STATUS "Using generated Hosted shared SDMMC lifecycle source")
