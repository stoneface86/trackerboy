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
 * [libtrackerboy](https://github.com/stoneface86/libtrackerboy)
 * [RtMidi](https://github.com/thestk/rtmidi)
 * Qt v5.12.10 or higher

Miniaudio and libtrackerboy are acquired via submodule so there is no need to
install these libraries on your system. All of the other requirements can be
acquired from vcpkg, or you can provide your own if that is preferred.

# Recommended build guide

## 1. Install Qt / dependencies

You will need to install the Qt5 development libraries. Note that Qt5 is not
listed as a dependency in the vcpkg manifest as there are better ways of
acquiring it. Here are a couple ways to get it:
 * Install via the official Qt installer (requires Qt account)
 * Install via [aqtinstall](https://github.com/miurahr/aqtinstall)
 * Install via your OS's package manager
 * Install via vcpkg (slow, not recommended)

 I do not recommend installing from vcpkg as it builds from source, which can
 take hours depending on your machine. (Also requires several GB of disk space).

I recommend using your system's package manager or aqtinstall if your system
does not have a package manager.

Depending on how you installed qt, you may have to set CMAKE_PREFIX_PATH to
your Qt installation when configuring, simply pass
`-DCMAKE_PREFIX_PATH="path/to/qt"` when running cmake. See these
[instructions](https://doc.qt.io/qt-5/cmake-get-started.html) for more details.

## 2. Clone the repository

```sh
git clone --recursive https://github.com/stoneface86/trackerboy
cd trackerboy
```

## 3. Build

Configure using cmake for a release or debug build.

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

For vcpkg users, specify the toolchain file from your vcpkg installation when
configuring:
```sh
... -DCMAKE_TOOLCHAIN_FILE="<VCPKG_ROOT>/scripts/buildsystems/vcpkg.cmake"
```

## 4. (Optional) install

Depending on your CMAKE_INSTALL_PREFIX, you may need administrator priviledges
```sh
cmake --build build --target install
```

## Options

Here is a table of options that can be used when building.

| Option            | Type | Default | Description                                         |
|-------------------|------|---------|-----------------------------------------------------|
| BUILD_TESTING     | BOOL | ON      | Enables unit testing                                |
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
