cmake_minimum_required(VERSION 3.8)
project(gbsynth C)

#include_directories(
#	"include"
#)

set(GBSYNTH "gbsynth")
set(GBSYNTH_SRC "src/gbsynth.c" "src/square.c" "src/envelope.c")

add_library(${GBSYNTH} STATIC ${GBSYNTH_SRC})

target_include_directories(
	${GBSYNTH}
	PUBLIC
	"include"
)