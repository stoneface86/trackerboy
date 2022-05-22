
# project for the C to Nim bridge, for calling Nim code from C/C++

project(bridge C)

find_package(Nim REQUIRED)
set(NIMBLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/nimble")

execute_process(
    COMMAND "${NIM}" e "${CMAKE_CURRENT_SOURCE_DIR}/configure.nims" "${NIMBLE}" "${NIMBLE_DIR}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Nim should use the same compiler as cmake
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(NIM_CC "--cc:vcc")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(NIM_CC "--cc:gcc")
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    # Clang or AppleClang
    set(NIM_CC "--cc:clang")
else ()
    # unknown compiler family, might want to abort here
    # or provide your own compiler config (see nim.cfg)
    message(FATAL_ERROR "unknown compiler, manual configuration required")
endif ()

if (WIN32)
    set(BRIDGE_LIB "bridge.lib")
else ()
    set(BRIDGE_LIB "libbridge.a")
endif ()
set(BRIDGE_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/${BRIDGE_LIB}")

set(BRIDGE_MODULE "bridge.nim")
set(BRIDGE_DEPS "")
set(BRIDGE_NIMCACHE "${CMAKE_CURRENT_BINARY_DIR}/nimcache")


# build the library via nimble
add_custom_command( 
    OUTPUT "${BRIDGE_LIB_PATH}"
    COMMAND "${NIMBLE}" --nimbleDir:"${NIMBLE_DIR}" c --app:staticlib --nomain ${NIM_CC}
    --out:"${BRIDGE_LIB_PATH}" --outdir:"${CMAKE_CURRENT_BINARY_DIR}" --nimcache:"${BRIDGE_NIMCACHE}"
    -d:cmakeBuildType:${CMAKE_BUILD_TYPE} "${BRIDGE_MODULE}"
    DEPENDS "${BRIDGE_MODULE}" "${BRIDGE_DEPS}" config.nims cmake.nim bridge.nimble
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_target(
    nimbuild 
    DEPENDS "${BRIDGE_LIB_PATH}"
    COMMENT "Building Nim bridge"
)

add_library(bridge STATIC IMPORTED GLOBAL)
set_target_properties(bridge PROPERTIES IMPORTED_LOCATION "${BRIDGE_LIB_PATH}")

add_dependencies(bridge nimbuild)
