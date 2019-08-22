# Building

Use the main CMakeLists.txt to build the library. The ui and demo programs are optional,
and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++11 standard or later
 * CMake
 * Portaudio
 * Qt 5

Portaudio is included in the repo as a git submodule, to use add `--recursive` when cloning
this repo or do a `git submodule update --init` before running CMake.

In order to build the main ui, you will need to install Qt5 on your development machine.
