# Building

Use the main CMakeLists.txt to build the library. The ui and demo programs are optional,
and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake
 * Portaudio
 * wxWidgets (version 3.0.0 and later)

Portaudio is included in the repo as a git submodule, to use add `--recursive` when cloning
this repo or do a `git submodule update --init` before running CMake.

In order to build the main ui, you will need to install wxWidgets on your development machine.
Windows users may need to specify the install location manually when configuring. (cmake variables
wxWidgets_LIB_DIR and wxWidgets_ROOT_DIR). 

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0

If you have issues compiling the project using a compiler not listed here, please submit an
issue.
