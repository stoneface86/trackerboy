cmake_minimum_required(VERSION 3.8)
project(gbsynth CXX)

#include_directories(
#	"include"
#)

set(GBSYNTH "gbsynth")
set(GBSYNTH_SRC 
	"src/gbsynth.cpp" 
	"src/Channel.cpp" 
	"src/EnvChannel.cpp"
	"src/SquareChannel.cpp"
)

add_library(${GBSYNTH} STATIC ${GBSYNTH_SRC})

target_include_directories(
	${GBSYNTH}
	PUBLIC
	"include"
)