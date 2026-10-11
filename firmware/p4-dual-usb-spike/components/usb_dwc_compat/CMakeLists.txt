idf_component_register(
    SRCS "../../../p4-core/components/usb_storage/usb_dwc_hal_compat.c"
         "../../../p4-core/components/usb_storage/usb_bus_trace.c"
    INCLUDE_DIRS "../../../p4-core/components/usb_storage/include"
    REQUIRES hal esp_timer
    PRIV_REQUIRES esp_mm
)

target_link_options(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=usb_dwc_hal_chan_decode_intr"
    "-Wl,--wrap=usb_dwc_hal_decode_intr"
    "-Wl,--wrap=usb_dwc_hal_chan_activate"
    "-Wl,--wrap=esp_cache_msync")
