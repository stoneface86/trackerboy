project(demo CXX)

set(BASE "base")
set(BASE_SRC "base.cpp")

add_library(${BASE} OBJECT EXCLUDE_FROM_ALL ${BASE_SRC})
target_include_directories(${BASE} PUBLIC ${INC_DIR})

function(add_demo demoName)
	add_executable(
		${demoName}
		${demoName}.cpp
		$<TARGET_OBJECTS:${BASE}>
	)
	target_link_libraries(
		${demoName}
		${GBSYNTH}
		portaudio_static
	)
endfunction(add_demo)