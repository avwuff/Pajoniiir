# Invoked at configure time and before every incremental component build.
if(NOT GIT_EXECUTABLE)
    find_package(Git REQUIRED)
endif()
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${PAJONIIIR_IDENTITY_SOURCE_DIR}"
    RESULT_VARIABLE source_result OUTPUT_VARIABLE PAJONIIIR_SOURCE_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(LENGTH "${PAJONIIIR_SOURCE_SHA}" sha_bytes)
if(NOT source_result EQUAL 0 OR NOT sha_bytes EQUAL 40 OR
   NOT PAJONIIIR_SOURCE_SHA MATCHES "^[0-9a-f]+$")
    message(FATAL_ERROR "A verified Git source SHA is required")
endif()
execute_process(COMMAND "${GIT_EXECUTABLE}" status --porcelain
    WORKING_DIRECTORY "${PAJONIIIR_IDENTITY_SOURCE_DIR}"
    RESULT_VARIABLE source_result OUTPUT_VARIABLE source_dirty)
if(NOT source_result EQUAL 0)
    message(FATAL_ERROR "Cannot verify source cleanliness")
endif()
if(PAJONIIIR_RELEASE_BUILD AND source_dirty)
    message(FATAL_ERROR "Release builds require a clean committed source")
endif()
set(PAJONIIIR_SOURCE_DIRTY 0)
if(source_dirty)
    set(PAJONIIIR_SOURCE_DIRTY 1)
endif()
# configure_file preserves timestamps when content is unchanged.
configure_file("${PAJONIIIR_IDENTITY_SOURCE_DIR}/board_build_identity_generated.h.in"
    "${PAJONIIIR_IDENTITY_HEADER}" @ONLY)
configure_file("${PAJONIIIR_IDENTITY_SOURCE_DIR}/board_build_identity.json.in"
    "${PAJONIIIR_IDENTITY_JSON}" @ONLY)
