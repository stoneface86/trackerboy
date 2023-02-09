
version         = "0.6.3"
author          = "stoneface"
description     = "Game Boy / Game Boy Color music tracker"
license         = "MIT"

srcDir          = "src"
binDir          = "bin"
bin             = @["trackerboy"]
backend         = "cpp"

requires "nim >= 1.6.0"
requires "https://github.com/stoneface86/libtrackerboy#v0.7.1"

when defined(nimdistros):
    import std/distros
    if detectOs(Ubuntu):
        foreignDep("libasound2-dev")
        foreignDep("qt6-base")
    elif detectOs(ArchLinux):
        foreignDep("alsa-lib")
        foreignDep("qtbase6-dev")
    elif detectOs(MacOSX):
        foreignDep("qt6")
