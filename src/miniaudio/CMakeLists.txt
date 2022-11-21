
project(miniaudio LANGUAGES C)

add_library(miniaudio STATIC "miniaudio.c")
target_include_directories(miniaudio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>")

target_compile_definitions(miniaudio PUBLIC
    -DMA_NO_DECODING
    -DMA_NO_ENCODING
    -DMA_NO_GENERATION
    -DMA_NO_CUSTOM
    -DMA_NO_RESOURCE_MANAGER
    -DMA_NO_NODE_GRAPH
    -DMA_NO_ENGINE
)

# miniaudio dependencies
# windows: none
# mac: none
# linux: pthreads, libm, libdl
# bsd: pthreads, libm

if (UNIX AND NOT APPLE)
   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)
   find_library(LIB_M m)
   if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
       find_library(LIB_DL dl)
       target_link_libraries(miniaudio PRIVATE ${LIB_DL})
   endif ()
   target_link_libraries(miniaudio PRIVATE Threads::Threads ${LIB_M})
endif ()
