if(CONFIG_PAJONIIIR_DJ_LINK_SERVICE AND NOT CONFIG_PAJONIIIR_BOARD_JC1060 AND
   NOT PAJONIIIR_LINK_EXPERIMENT)
    message(FATAL_ERROR "Network service requires an explicit non-production Link experiment")
endif()
if(CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM AND
   (NOT CONFIG_PAJONIIIR_SD_INTERNAL_BOUNCE OR NOT
    ((CONFIG_PAJONIIIR_BOARD_JC1060 AND PAJONIIIR_STORAGE_EXPERIMENT) OR
     (CONFIG_PAJONIIIR_BOARD_M3 AND CONFIG_BSP_M3_HS_CONTROLLER AND
      CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC AND PAJONIIIR_S2_HS_EXPERIMENT))))
    message(FATAL_ERROR "USB PSRAM DMA requires a qualified board experiment and internal SD bounce")
endif()
idf_component_register(SRCS "board_sd_dma.c" "board_build_identity.c" INCLUDE_DIRS "include"
    REQUIRES board_capabilities esp_lcd esp_lcd_touch esp_codec_dev esp_driver_i2c
             esp_driver_i2s esp_driver_gpio esp_driver_ledc esp_driver_sdmmc
             fatfs sdmmc esp_hw_support
    PRIV_REQUIRES esp_timer)

find_package(Git REQUIRED)
set(PAJONIIIR_IDENTITY_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(PAJONIIIR_IDENTITY_HEADER "${CMAKE_CURRENT_BINARY_DIR}/board_build_identity_generated.h")
set(PAJONIIIR_IDENTITY_JSON "${CMAKE_BINARY_DIR}/board_build_identity.json")
set(identity_generator "${CMAKE_CURRENT_LIST_DIR}/../../cmake/generate_board_identity.cmake")
include("${identity_generator}")
add_custom_target(board_build_identity_refresh
    COMMAND "${CMAKE_COMMAND}"
        "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
        "-DPAJONIIIR_IDENTITY_SOURCE_DIR=${PAJONIIIR_IDENTITY_SOURCE_DIR}"
        "-DPAJONIIIR_IDENTITY_HEADER=${PAJONIIIR_IDENTITY_HEADER}"
        "-DPAJONIIIR_IDENTITY_JSON=${PAJONIIIR_IDENTITY_JSON}"
        "-DPAJONIIIR_RELEASE_BUILD=${PAJONIIIR_RELEASE_BUILD}"
        -P "${identity_generator}"
    BYPRODUCTS "${PAJONIIIR_IDENTITY_HEADER}" "${PAJONIIIR_IDENTITY_JSON}"
    VERBATIM)
add_dependencies(${COMPONENT_LIB} board_build_identity_refresh)
target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
