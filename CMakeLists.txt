cmake_minimum_required(VERSION 3.8)
project(gbsynth C CXX)

#include_directories(
#	"include"
#)

set(GBSYNTH "gbsynth")
set(GBSYNTH_SRC "src/gbsynth.c" "src/square.c" "src/envelope.c")

set(GBSYNTH2 "gbsynth2")
set(GBSYNTH2_SRC 
	"src/gbsynth2.cpp" 
	"src/Channel.cpp" 
	"src/EnvChannel.cpp"
	"src/SquareChannel.cpp"
)

add_library(${GBSYNTH} STATIC ${GBSYNTH_SRC})
add_library(${GBSYNTH2} STATIC ${GBSYNTH2_SRC})

target_include_directories(
	${GBSYNTH}
	PUBLIC
	"include"
)

target_include_directories(
	${GBSYNTH2}
	PUBLIC
	"include"
)