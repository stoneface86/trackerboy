
import trackerboy/platforms

# check that we have a supported platform and compiler.

when @pLinux:
    when not defined(gcc):
        {. error: "Must use GCC for linux platform: compile with --cc:gcc" .}
elif @pMac:
    when not defined(clang):
        {. error: "Must use Clang for mac platform: compile with --cc:clang" .}
elif @pWindows:
    when not defined(vcc):
        {.error: "Must use MSVC for windows platform: compile with --cc:vcc" .}
else: # platform == pUnknown
    {.error: "Cannot build for this platform: platform is unknown" .}

import build/utils

# vendored dependencies

import build/vendor

# Qt Configuration

import build/qtconf

const config = block:
    var res = merge(vendorConfig, qtConfig)
    when defined(vcc):
        res.lflags.insert("/link", 0)
    res

static: echo config
config.apply()

# Main program logic

import trackerboy/main
import std/exitprocs

setProgramResult( main() )

# type
#     ccstringImpl {.importc: "const char*".} = cstring
#     ccstring = distinct ccstringImpl

# proc maVersionString(): ccstring
#     {. importc: "ma_version_string", header: "miniaudio.h" .}

# echo "Miniaudio version: ", maVersionString().cstring
