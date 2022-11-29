
project(vendored)

add_library(vendoredIncludes INTERFACE)
target_include_directories(vendoredIncludes INTERFACE include)

add_subdirectory(miniaudio)
