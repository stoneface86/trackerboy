# Building

Use the main CMakeLists.txt to build the library. The ui and demo programs are
optional, and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake 3.14 or higher
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [gbapu](https://github.com/stoneface86/gbapu)
 * [RtMidi](https://github.com/thestk/rtmidi)
 * Qt v5.12.10<sup>[1](#qtversion-footnote)</sup>

Linux requirements:
 * ALSA development libraries (ie libasound-dev) - required by RtMidi

Optional:
 * JACK - for RtMidi

Miniaudio, RtMidi and gbapu will be acquired via cmake's FetchContent command
so there is no need to install these libraries on your system. In order to
build the main ui, you will need to install Qt 5 on your development machine.
I recommend using [aqtinstall](https://github.com/miurahr/aqtinstall) for
windows as it does not require a Qt account unlike the official Qt installer.
Make sure you set the CMAKE_PREFIX_PATH variable to where you installed Qt so
that cmake finds it when configuring. See these
[instructions](https://doc.qt.io/qt-5/cmake-get-started.html) for more details.

<a name="qtversion-footnote">1</a>: You can build with any later Qt 5 version,
but all code in this repo must be compatible with v5.12.10

## General instructions

Install all required dependencies and then run cmake

```sh
git clone https://github.com/stoneface86/trackerboy
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

| Option                  | Type | Description                                                 |
|-------------------------|------|-------------------------------------------------------------|
| TRACKERBOY_ENABLE_DEMO  | BOOL | If enabled, the demo programs will be built. Default is OFF |
| TRACKERBOY_ENABLE_TESTS | BOOL | Enables unit tests. Default is ON                           |
| TRACKERBOY_ENABLE_UI    | BOOL | Enables building of the trackerboy ui. Default is ON        |

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0
 * GCC 10.2.0

If you have issues compiling the project using a compiler not listed here,
please submit an issue.
