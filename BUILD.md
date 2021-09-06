# Building

UPDATE: Only windows and linux builds are supported at this point in time.
I simply do not have the resources to build and test for all platforms.
If you have any issues building on macOS, feel free to submit an issue or
pull request. Contributions are always welcome.

Use the main CMakeLists.txt to build the library. The ui and demo programs are
optional, and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake 3.9 or higher
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [gbapu](https://github.com/stoneface86/gbapu)
 * [RtMidi](https://github.com/thestk/rtmidi)
 * (Optional) [Catch2](https://github.com/catchorg/Catch2)
 * Qt v5.12.10 or higher

Miniaudio and gbapu are acquired via submodule so there is no need to install
these libraries on your system. All of the other requirements can be acquired
from vcpkg, or you can provide your own if that is preferred. Some action on
your part may be necessary when not using vcpkg.

Catch2 is only required for testing. Catch2 is not needed if ENABLE_TESTS is
set to OFF.

# Recommended build guide

## 1. Clone the repository

```sh
git clone --recursive https://github.com/stoneface86/trackerboy
```

## 2. Install Qt

If you are building the ui, you will need to install Qt5. Note that Qt5 is not
listed as a dependency in the vcpkg manifest as there are better ways of
acquiring it. Here are a couple ways to get it:
 * Install via the official Qt installer (requires Qt account)
 * Install via [aqtinstall](https://github.com/miurahr/aqtinstall) (does not require Qt account)
 * Install via your OS's package manager
 * Install via vcpkg (slow, not recommended)

If you choose to install via vcpkg, you will need to bootstrap the one provided
in the submodule first before configuring. I do not recommend installing from
vcpkg as it builds from source, which can take hours depending on your machine.
(Also requires several GB of disk space).

I recommend aqtinstall, as it's simple to use and is the most flexible. Simply
choose the version you want and where to install it. Easy, and works on all
platforms.

Depending on how you installed qt, you may have to set CMAKE_PREFIX_PATH to
your Qt installation when configuring, simply pass
`-DCMAKE_PREFIX_PATH="path/to/qt"` when running cmake. See these
[instructions](https://doc.qt.io/qt-5/cmake-get-started.html) for more details.

## 3. Configure

The rest of the required libraries are acquired via [vcpkg](https://vcpkg.io/en/index.html).
Manifest mode is used, so all you need to do is run cmake with the vcpkg
toolchain:

```sh
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" ../
```

You should then be able to build the project.

### Notes

You may need to install the ALSA development libraries when building on linux

For apt based distros:
```sh
sudo apt install libasound2-dev
```

# Alternative building

When not using vcpkg, you must provide the cmake packages for RtMidi (and optionally, Catch2)
yourself. To do so you must acquire the source, build and install using cmake. Then either define
CMAKE_PREFIX_PATH or <package_name>_DIR with the location of the installed packages.

Or, you can enable the FIND_RTMIDI option and cmake will search your system
for RtMidi installed on your system. The RtMidi find script is located in the
[cmake](./cmake) folder.

## Options

Here is a table of options that can be used when building.

| Option       | Type | Default | Description                                         |
|--------------|------|---------|-----------------------------------------------------|
| ENABLE_DEMO  | BOOL | OFF     | If enabled, the demo programs will be built.        |
| ENABLE_TESTS | BOOL | ON      | Enables unit testing                                |
| ENABLE_UI    | BOOL | ON      | Enables building of the trackerboy ui               |
| ENABLE_UNITY | BOOL | OFF     | Enables unity builds (requires cmake 3.16)          |
| FIND_RTMIDI  | BOOL | OFF     | If enabled, cmake will search the system for RtMidi |

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
