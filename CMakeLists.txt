cmake_minimum_required(VERSION 3.8)
project(gbsynth)

set(INC_DIR "include")

option(ENABLE_TESTS "Enable unit tests" ON)
option(ENABLE_DEMO "Enable building of demo programs (requires portaudio)" OFF)
option(ENABLE_SOUNDTEST "Enable building of sound test gui (requires Qt, portaudio)" OFF)

if (ENABLE_DEMO OR ENABLE_SOUNDTEST)
	add_subdirectory(portaudio EXCLUDE_FROM_ALL)
endif ()


set(GBSYNTH "gbsynth")
set(GBSYNTH_SRC 
	"src/Channel.cpp" 
	"src/EnvChannel.cpp"
	"src/gbsynth.cpp" 
	"src/Mixer.cpp"
	"src/NoiseChannel.cpp"
	"src/PulseChannel.cpp"
	"src/Sequencer.cpp"
	"src/SweepPulseChannel.cpp"
	"src/Synth.cpp"
	"src/tables.cpp"
	"src/WaveChannel.cpp"
)

add_library(${GBSYNTH} STATIC ${GBSYNTH_SRC})

target_include_directories(
	${GBSYNTH}
	PUBLIC
	${INC_DIR}
)

if (ENABLE_DEMO)
	add_subdirectory(demo)
endif (ENABLE_DEMO)

if (ENABLE_SOUNDTEST)
#	add_subdirectory(soundtest)
endif (ENABLE_SOUNDTEST)

