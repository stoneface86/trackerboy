project(demo CXX)

set(DEMO "demo")
set(DEMO_SRC
	"main.cpp"
	"Demo.cpp"
	"demos/DutyDemo.cpp"
	"demos/SweepDemo.cpp"
	"demos/WaveDemo.cpp"
)

add_executable(${DEMO} ${DEMO_SRC})
#target_include_directories(${DEMO} PUBLIC ${INC_DIR})
target_link_libraries(${DEMO} ${GBSYNTH} portaudio_static)