cmake_minimum_required(VERSION 3.20)
project(pajoniiir_ui_next_sim C)

# Headless renderer of the new DJ UI. Writes one PPM per frame.
# cmake -S tests/ui_next_sim -B <build> -DLVGL_DIR=<LVGL 9.5 checkout>
if(NOT LVGL_DIR)
    message(FATAL_ERROR "LVGL_DIR must point to an LVGL 9.5 checkout")
endif()

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LV_BUILD_CONF_PATH "${REPO_ROOT}/tests/ui_simulator/lv_conf.h" CACHE FILEPATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
add_subdirectory("${LVGL_DIR}" "${CMAKE_BINARY_DIR}/lvgl" EXCLUDE_FROM_ALL)

set(NEXT "${REPO_ROOT}/firmware/p4-core/components/ui_next")
file(GLOB NEXT_FONTS "${NEXT}/fonts/*.c")
add_executable(ui_next_sim main.c
    "${NEXT}/ui_next_layout.c" "${NEXT}/ui_next_theme.c" "${NEXT}/ui_next_wave.c"
    "${NEXT}/ui_next_demo.c" "${NEXT}/ui_next_util.c" "${NEXT}/ui_next_chrome.c" "${NEXT}/ui_next_perform.c"
    "${NEXT}/ui_next_boot.c" "${NEXT}/ui_next_browse.c" "${NEXT}/ui_next_search.c" "${NEXT}/ui_next_alerts.c" "${NEXT}/ui_next_trackinfo.c" "${NEXT}/ui_next_source.c" "${NEXT}/ui_next_fx.c" "${NEXT}/ui_next_record.c" "${NEXT}/ui_next_settings.c" ${NEXT_FONTS})
target_include_directories(ui_next_sim PRIVATE "${NEXT}/include")
target_compile_options(ui_next_sim PRIVATE -Wall -Wextra -g)
target_link_libraries(ui_next_sim PRIVATE lvgl m)
