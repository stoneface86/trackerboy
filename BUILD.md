# Building

Use the main CMakeLists.txt to build the library. The ui and demo programs are
optional, and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [gbapu](https://github.com/stoneface86/gbapu)
 * Qt v5.12.10<sup>[2](#qtversion-footnote)</sup>

Both miniaudio and gbapu are included in the repo as git submodules, to use
add `--recursive` when cloning this repo or do a `git submodule update --init`
before running CMake.

In order to build the main ui, you will need to install Qt 5 on your development
machine. Windows users may need to specify the install location manually when
configuring. (cmake variable Qt5_DIR).

<a name="qtversion-footnote">2</a>: You can build with any later Qt 5 version, but all code in this repo must
be compatible with v5.12.10

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

For unit tests, just run ctest in the build directory to run all tests.
Individual tests are usually named after their respective class (ie DataItem,
Module, etc) or filename.
```sh
ctest
```

## Options

Here is a table of options that can be used when building.

| Option                  | Type | Description                                                                       |
|-------------------------|------|-----------------------------------------------------------------------------------|
| TRACKERBOY_ENABLE_DEMO  | BOOL | If enabled, the demo programs will be built. Default is OFF                       |
| TRACKERBOY_ENABLE_TESTS | BOOL | Enables unit tests. Default is ON                                                 |
| TRACKERBOY_ENABLE_UI    | BOOL | Enables building of the trackerboy ui. Default is ON                              |

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0
 * GCC 10.2.0

If you have issues compiling the project using a compiler not listed here,
please submit an issue.

## Building on Windows

If CMake cannot find your Qt5 installation, you must set it via the `Qt5_DIR`
variable. This variable should be the path to where you installed Qt
(ie C:\Qt\5.15.0\msvc2019_64).
