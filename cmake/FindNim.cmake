# finds the nim compiler, adapted from https://gist.github.com/shiva/521536ddea6425025606

find_program(NIM nim)
find_program(NIMBLE nimble)
mark_as_advanced(NIM)
mark_as_advanced(NIMBLE)

if (NIM)
    execute_process(COMMAND ${NIM} "--version" OUTPUT_VARIABLE NIM_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" NIM_VERSION ${NIM_VERSION})
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Nim
    REQUIRED_VARS NIM NIMBLE
    VERSION_VAR NIM_VERSION
)

