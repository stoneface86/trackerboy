cmake_minimum_required(VERSION 3.8)
project(gbsynth C CXX)

#include_directories(
#	"include"
#)

set(GBSYNTH2 "gbsynth2")
set(GBSYNTH2_SRC 
	"src/gbsynth2.cpp" 
	"src/Channel.cpp" 
	"src/EnvChannel.cpp"
	"src/SquareChannel.cpp"
)

add_library(${GBSYNTH2} STATIC ${GBSYNTH2_SRC})

target_include_directories(
	${GBSYNTH2}
	PUBLIC
	"include"
)