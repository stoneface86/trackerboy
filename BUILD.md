# Building

UPDATE: Only windows and linux builds are supported at this point in time.
I simply do not have the resources to build and test for all platforms.
If you have any issues building on macOS, feel free to submit an issue or
pull request. Contributions are always welcome.

Use the main CMakeLists.txt to build the library. The ui and demo programs are
optional, and can be enabled/disabled by setting their respective options.

Requirements:
 * C/C++ compiler with the C++17 standard or later
 * CMake 3.8 or higher
 * [Miniaudio](https://github.com/mackron/miniaudio)
 * [gbapu](https://github.com/stoneface86/gbapu)
 * [RtAudio](https://github.com/thestk/rtaudio)
 * [RtMidi](https://github.com/thestk/rtmidi)
 * Qt v5.12.10 or higher

Miniaudio is included in the repo and gbapu is available via submodule
so there is no need to install these libraries on your system. All of the
other requirements can be acquired from vcpkg, or you can provide your own if
that is preferred. If you choose to not use vcpkg, then cmake will attempt
to find all required dependencies installed on your system.

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
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" ../
```

You should then be able to build the project.

### Notes

If using vcpkg, the rtmidi port builds without support for ALSA (port file defines
RTMIDI_API_ALSA=OFF). So if you are using vcpkg on linux, the ALSA MIDI api will not
be usable. Not sure why they disabled it when the rtaudio port builds with ALSA support.
You will need to edit the port file if you want ALSA support, by commenting out the
line with `OPTIONS -DRTMIDI_API_ALSA=OFF`

# Alternative building

When not using vcpkg, you must provide the cmake packages for RtAudio and RtMidi yourself.
To do so you must acquire the source, build and install using cmake. Then either define
CMAKE_PREFIX_PATH or <package_name>_DIR with the location of the installed packages.

Or, you can define `USE_FIND_SCRIPTS` and cmake will search your system for the required
dependencies. The scripts used are located in the cmake folder.

## Options

Here is a table of options that can be used when building.

| Option       | Type | Default | Description                                  |
|--------------|------|---------|----------------------------------------------|
| ENABLE_DEMO  | BOOL | OFF     | If enabled, the demo programs will be built. |
| ENABLE_TESTS | BOOL | ON      | Enables unit testing                         |
| ENABLE_UI    | BOOL | ON      | Enables building of the trackerboy ui        |

## Compilers

Here is a list of compilers that successfully build the project:
 * MSVC v19.27
 * MSVC v19.26
 * GCC 7.5.0
 * GCC 10.2.0

If you have issues compiling the project using a compiler not listed here,
please submit an issue.
