# Building

Use the main CMakeLists.txt to build the application. See below for available
build options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake 3.12 or higher
 * (Linux only) ALSA development libraries
 * Qt v6.0.0 or higher
 * x86_64 system, supported OSes: Linux, MacOS, Windows

Dependencies satisfied by CMake:
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [libtrackerboy](https://github.com/stoneface86/libtrackerboy/tree/cpp-last)
 * [RtMidi](https://github.com/thestk/rtmidi)

Miniaudio source is included in this repo, also libtrackerboy and RtMidi 
are acquired via CPM.cmake, so there is no need to install these libraries.

For detailed instructions for your OS, follow one of these build guides:
 * [Linux](#linux-build-guide)
 * [MacOS](#macos-build-guide)
 * [Windows](#windows-build-guide)

It is assumed you already have the essentials installed (C++ compiler, git), so
the installation of these will not be covered by these guides.

# Linux build guide

## 1. Install Dependencies

You can also install Qt via [aqtinstall](https://github.com/miurahr/aqtinstall)

Arch:
```sh
sudo pacman -Sy --needed alsa-lib cmake qt6-base
```

Ubuntu:
```sh
sudo apt update
sudo apt install libasound2-dev cmake qtbase6-dev
```

For other distros the package manager and package names may differ, but the
process is the same. Install ALSA development libraries, CMake and Qt5 base
(Trackerboy only uses QtWidgets, QtGui and QtCore).

## 2. Get trackerboy

```sh
git clone https://github.com/stoneface86/trackerboy
cd trackerboy
```

## 3. Configure

For a release build (users):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_UNITY=ON
```

For a debug build (developers):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

## 4. Build

```sh
cmake --build build --target all
```

If successful you can run Trackerboy via:
```sh
./build/src/trackerboy
```

# MacOS build guide

This guide assumes you have [brew](https://brew.sh) installed.

## 1. Install dependencies

```sh
brew install cmake qt6
```

## 2. Get trackerboy

Same as the [linux instructions](#2-get-trackerboy)

## 3. Configure

For a release build (users):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_UNITY=ON
```

For a debug build (developers):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

## 4. Build

Same as the [linux instructions](#4-build)

# Windows build guide

## 1. Install Qt

Use [aqtinstall](https://github.com/miurahr/aqtinstall), the official installer
asks for a user account. You will need pip installed.

```sh
pip install -U pip
pip install aqtinstall
```

Now get your desired version of Qt using `aqt`, install this to a directory of
your choice which I will refer to as `OUT`. `VERSION` is the Qt version to
install, and `ARCH` is the target architecture (32-bit or 64-bit + msvc or
mingw). If you are unsure which one to pick, then run
`aqt list-qt windows desktop --arch VERSION` for a list of available choices.

Example: for Qt 5.15.2, 64-bit MSVC:
```sh
aqt install-qt windows desktop 5.15.2 win64_msvc2019_64 --outputdir OUT
```

Example: for Qt 5.15.2, 32-bit MSVC:
```sh
aqt install-qt windows desktop 5.15.2 win32_msvc2019_64 --outputdir OUT
```

When configuring CMake, you will need to pass this path to variable Qt5_DIR,
substituting OUT, VERSION and ARCH with your selections:
```
OUT/VERSION/ARCH/lib/cmake/Qt5
```

Make note of this path, the guide will refer to this path as QTDIR

## 2. Get trackerboy

Same as the [linux instructions](#2-get-trackerboy)

## 3. Configure

When configuring you will need to pass your QTDIR location to
cmake as variable Qt5_DIR.

For a release build (users):
```sh
cmake -S . -B build -DQt5_DIR=QTDIR -DCMAKE_BUILD_TYPE=Release -DENABLE_UNITY=ON
```

For a debug build (developers):
```sh
cmake -S . -B build -DQt5_DIR=QTDIR -DCMAKE_BUILD_TYPE=Debug
```

## 4. Build

Same as the [linux instructions](#4-build)


# (Optional) Install

You can install the built binary if desired. Depending on your
CMAKE_INSTALL_PREFIX, you may need administrator priviledges.
```sh
cmake --build build --target install
```

# Options

Here is a table of options that can be used when building.

| Option            | Type | Default | Description                                         |
|-------------------|------|---------|-----------------------------------------------------|
| BUILD_TESTING     | BOOL | OFF     | Enables unit testing                                |
| ENABLE_UNITY      | BOOL | OFF     | Enables unity builds (requires cmake 3.16)          |
| ENABLE_DEPLOYMENT | BOOL | OFF     | Enables the deploy target                           |

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
