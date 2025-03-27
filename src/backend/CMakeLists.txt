
project(backend C)

find_package(Nim REQUIRED)
set(NIMBLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/nimble")

# Nim should use the same compiler as cmake
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(NIM_CC "vcc")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(NIM_CC "gcc")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Clang or AppleClang
  set(NIM_CC "clang")
else ()
  # unknown compiler family, might want to abort here
  # or provide your own compiler config (see nim.cfg)
  message(FATAL_ERROR "unknown compiler, manual configuration required")
endif ()

# pass Qt6 include dirs to Nim
# list(JOIN Qt6Core_INCLUDE_DIRS ";" BACKEND_INCLUDES)
# list(JOIN Qt6Gui_INCLUDE_DIRS ";" BACKEND_INCLUDES)
# list(JOIN Qt6Widgets_INCLUDE_DIRS ";" BACKEND_INCLUDES)
# list(REMOVE_DUPLICATES BACKEND_INCLUDES)
# foreach (dir IN ITEMS ${BACKEND_INCLUDES})
#   string(APPEND BACKEND_INCLUDES_CODE "switch \"cincludes\", \"${dir}\"\n")
# endforeach ()

set(BACKEND_LIB "${CMAKE_STATIC_LIBRARY_PREFIX}backend${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(BACKEND_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/${BACKEND_LIB}")

set(BACKEND_IN_NIMBLE "backend.nimble")
set(BACKEND_IN_CONFIG "config.nims")
set(BACKEND_PROJECT "backend.nim")
set(BACKEND_PROJECT_SHIM "${CMAKE_CURRENT_BINARY_DIR}/${BACKEND_PROJECT}")
set(BACKEND_NIMCACHE "${CMAKE_CURRENT_BINARY_DIR}/nimcache")
configure_file("${BACKEND_IN_NIMBLE}" "${CMAKE_CURRENT_BINARY_DIR}/backend.nimble" @ONLY)
configure_file("${BACKEND_IN_CONFIG}" "${CMAKE_CURRENT_BINARY_DIR}/config.nims" @ONLY)
file(GENERATE OUTPUT "${BACKEND_PROJECT_SHIM}" CONTENT "include \"${CMAKE_CURRENT_SOURCE_DIR}/${BACKEND_PROJECT}\"")

execute_process(
  COMMAND "${NIMBLE}" -y --nimbleDir:nimble setup
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Get the location of the nim standard library (needed so we can include nimbase.h)
execute_process(COMMAND 
  "${NIM}" --hints:off --eval:"import std/compilesettings;echo querySetting(SingleValueSetting.libPath)"
  OUTPUT_VARIABLE NIM_LIB_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# output file that never gets built, ensures that nimble is always executed
# during build
set(BACKEND_ALWAYS_BUILD "${CMAKE_CURRENT_BINARY_DIR}/dummy")

add_custom_target(
  compileBackend
  DEPENDS "${BACKEND_ALWAYS_BUILD}"
  COMMENT "Compiling backend (Nim)"
)
add_custom_command(
  OUTPUT 
    "${BACKEND_ALWAYS_BUILD}"
    "${BACKEND_LIB_PATH}"
  COMMAND "${NIM}" cpp ${BACKEND_PROJECT}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Compiling backend (Nim)"
)


add_library(backend STATIC IMPORTED GLOBAL)
add_dependencies(backend compileBackend)
set_target_properties(backend PROPERTIES IMPORTED_LOCATION "${BACKEND_LIB_PATH}")
file(MAKE_DIRECTORY "${BACKEND_NIMCACHE}")
target_include_directories(backend INTERFACE 
  "${BACKEND_NIMCACHE}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(backendTest EXCLUDE_FROM_ALL "test/backendTest.cxx")
target_link_libraries(backendTest PRIVATE backend)

