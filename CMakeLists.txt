cmake_minimum_required(VERSION 3.8)
project(gbsynth CXX)

#include_directories(
#	"include"
#)

set(GBSYNTH "gbsynth")
set(GBSYNTH_SRC 
	"src/Channel.cpp" 
	"src/EnvChannel.cpp"
	"src/FreqChannel.cpp"
	"src/gbsynth.cpp" 
	"src/Mixer.cpp"
	"src/NoiseChannel.cpp"
	"src/PulseChannel.cpp"
	"src/Sequencer.cpp"
	"src/Sweep.cpp"
	"src/Synth.cpp"
	"src/tables.cpp"
	"src/WaveChannel.cpp"
)

add_library(${GBSYNTH} STATIC ${GBSYNTH_SRC})

target_include_directories(
	${GBSYNTH}
	PUBLIC
	"include"
)