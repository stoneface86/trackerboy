# Building

Use the main CMakeLists.txt to build the library. The ui and demo programs are optional,
and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake
 * Portaudio
 * Qt5

Portaudio is included in the repo as a git submodule, to use add `--recursive` when cloning
this repo or do a `git submodule update --init` before running CMake.

In order to build the main ui, you will need to install Qt5 on your development machine.
Windows users may need to specify the install location manually when configuring. (cmake variable
Qt5_DIR).

## General instructions

Install all required dependencies and then run cmake

```sh
git clone --recursive https://github.com/stoneface86/trackerboy
cd trackerboy
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

For unit tests, just run ctest in the build directory to run all tests. Individual tests are
usally named after their respective class (ie DataItem, Module, etc) or filename.
```
ctest
```

## Options

Here is a table of options that can be used when building.

| Option                  | Type | Description                                                                       |
|-------------------------|------|-----------------------------------------------------------------------------------|
| TRACKERBOY_ENABLE_DEMO  | BOOL | If enabled, the demo programs will be built. Default is OFF                       |
| TRACKERBOY_ENABLE_TESTS | BOOL | Enables unit tests. Default is ON                                                 |
| TRACKERBOY_ENABLE_UI    | BOOL | Enables building of the trackerboy ui. Default is ON                              |
| TRACKERBOY_COVERAGE     | BOOL | Builds unit tests with coverage when enabled. Only works with GCC. Requires lcov. Default is OFF |

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0

If you have issues compiling the project using a compiler not listed here, please submit an
issue.

## Building on Windows

Use the general instructions for building on windows, except you will need to install the DirectX SDK
for Portaudio. If Portaudio cannot find the SDK, set the `DXSDK_ROOT_DIR` variable to where you installed
it. Then make sure the `PA_USE_DS` variable is enabled. If you skip this step, you will not be able
to use the DirectX API for sound output in the ui.

If CMake cannot find your Qt5 installation, you must set it via the `Qt5_DIR` variable. This variable should
be the path to where you installed Qt (ie C:\Qt\5.15.0\msvc2019_64).
