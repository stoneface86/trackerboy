project(demo CXX)

set(BASE "base")
set(BASE_SRC "base.cpp")

add_library(${BASE} STATIC EXCLUDE_FROM_ALL ${BASE_SRC})
target_include_directories(${BASE} PUBLIC ${INC_DIR})
target_link_libraries(
	${BASE} 
	${GBSYNTH}
	portaudio_static
)

function(add_demo demoName)
	add_executable(
		${demoName}
		${demoName}.cpp
	)
	target_link_libraries(
		${demoName}
		${BASE}
	)
endfunction(add_demo)

add_demo(sweep)
add_demo(duty)
