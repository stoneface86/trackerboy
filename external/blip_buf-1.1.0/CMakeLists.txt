
project(blip_buf LANGUAGES C)

add_library(blip_buf STATIC "blip_buf.c")
target_include_directories(blip_buf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
