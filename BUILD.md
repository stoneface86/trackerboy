# Building

UPDATE: Only windows and linux builds are supported at this point in time.
I simply do not have the resources to build and test for all platforms.
If you have any issues building on macOS, feel free to submit an issue or
pull request. Contributions are always welcome.

Use the main CMakeLists.txt to build the library. The ui and demo programs are
optional, and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake 3.12 or higher
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [libtrackerboy](https://github.com/stoneface86/libtrackerboy)
 * [RtMidi](https://github.com/thestk/rtmidi) (4.0.0)
 * (Linux only) ALSA development libraries
 * Qt v5.12.10 or higher

Miniaudio, libtrackerboy and RtMidi are acquired via FetchContent so there is no
need to install these libraries on your system. You will however, need to
install Qt on your system.

Note: I am using my own fork of miniaudio (located [here](https://github.com/stoneface86/miniaudio-cmake))
that adds cmake support.

# Recommended build guide

Follow this guide to build Trackerboy.

## 1. Install Qt / dependencies

You will need to install the Qt5 development libraries.
Here are a couple ways to get it:
 * Install via the official Qt installer (requires Qt account)
 * Install via [aqtinstall](https://github.com/miurahr/aqtinstall)
 * Install via your OS's package manager

I recommend using your system's package manager or aqtinstall if your system
does not have a package manager.

Depending on how you installed qt, you may have to set CMAKE_PREFIX_PATH to
your Qt installation when configuring, simply pass
`-DCMAKE_PREFIX_PATH="path/to/qt"` when running cmake. See these
[instructions](https://doc.qt.io/qt-5/cmake-get-started.html) for more details.

For Linux builds, you will need to install the ALSA development libraries
(needed by RtMidi). For apt-based distros:
```sh
sudo apt install libasound2-dev
```

## 2. Clone the repository

```sh
git clone https://github.com/stoneface86/trackerboy
cd trackerboy
```

## 3. Build

Configure using cmake for a release or debug build. Debug is assumed if
you do not specify CMAKE_BUILD_TYPE.

For a release build (users):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_UNITY=ON
```

For a debug build (developers):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Then build:
```sh
cmake --build build --target all
```

## 4. (Optional) install

Depending on your CMAKE_INSTALL_PREFIX, you may need administrator priviledges
```sh
cmake --build build --target install
```

## Building without FetchContent

If you are unable to use FetchContent, you will need to acquire the source,
build and install for all of the required dependencies (miniaudio, libtrackerboy
and RtMidi). Then you will set `CMAKE_PREFIX_PATH` to the path of the installed
dependencies when configuring, along with `FETCH_DEPS=OFF`.

## Options

Here is a table of options that can be used when building.

| Option            | Type | Default | Description                                         |
|-------------------|------|---------|-----------------------------------------------------|
| BUILD_TESTING     | BOOL | ON      | Enables unit testing                                |
| ENABLE_UNITY      | BOOL | OFF     | Enables unity builds (requires cmake 3.16)          |
| ENABLE_DEPLOYMENT | BOOL | OFF     | Enables the deploy target                           |
| FETCH_DEPS        | BOOL | ON      | Acquires dependencies during configuration          |

Unity builds should only be used if you are just building trackerboy. It is
not recommended to have this enabled when developing.

Unity builds will be required for release builds to ensure space-efficient
binaries.

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0
 * GCC 10.2.0

If you have issues compiling the project using a compiler not listed here,
please submit an issue.
