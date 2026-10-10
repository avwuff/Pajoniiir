if(NOT IDF_TARGET STREQUAL "esp32p4" OR NOT CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH)
    message(FATAL_ERROR "P4 restart diagnostics require ESP32-P4 and flash ELF coredumps")
endif()

idf_component_register(SRCS "restart_diagnostics.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_system espcoredump riscv)
# External IDF calls are intercepted without editing the installed SDK.
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=panic_arch_fill_info"
    "-Wl,--wrap=esp_restart"
    "-Wl,--wrap=esp_restart_noos")

add_custom_target(p4_restart_capture_contract ALL
    COMMAND "${PYTHON}" "${CMAKE_CURRENT_LIST_DIR}/../../../../tools/check_restart_capture_elf.py"
        --build "${CMAKE_BINARY_DIR}"
        --output "${CMAKE_BINARY_DIR}/restart-capture-contract.json"
    COMMAND "${PYTHON}" "${CMAKE_CURRENT_LIST_DIR}/../../../../tests/restart_diagnostics/test_restart_capture_pipeline.py"
        --build "${CMAKE_BINARY_DIR}"
    DEPENDS gen_project_binary
    COMMENT "Checking actual P4 panic interception and coredump region"
    VERBATIM)
