# Preparation helper for a future licensed JC8012 entrypoint. Not included by
# current production projects. Do not add the overlay to p4-core/components.
function(pajoniiir_prepare_jc8012_ppa board output_variable)
    if(NOT "${board}" STREQUAL "jc8012p4a1")
        message(FATAL_ERROR "Private PPA preparation requires jc8012p4a1")
    endif()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    # A fresh per-configure directory avoids overwriting source or reusing a
    # partially generated component after a failed configure.
    string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef suffix)
    set(output "${CMAKE_BINARY_DIR}/jc8012-private-${suffix}/esp_driver_ppa")
    execute_process(
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../../../tools/prepare_jc8012_ppa.py"
            --idf "$ENV{IDF_PATH}" --output "${output}" --board "${board}"
        RESULT_VARIABLE result OUTPUT_QUIET ERROR_VARIABLE error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Pinned JC8012 PPA preparation failed: ${error}")
    endif()
    set(${output_variable} "${output}" PARENT_SCOPE)
endfunction()
