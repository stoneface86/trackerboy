
import std/strutils

const cmakeBuildType {.strdefine.} = "Debug"

type
    BuildType* = enum
        ## Enum of CMake default build types
        btDebug
            ## Debug mode, default if a type was not provided
        btRelease
            ## Release mode
        btMinSizeRel
            ## Release mode, optimizing for size
        btRelWithDebInfo
            ## Release mode with debug info
        btCustom
            ## Custom type

const
    buildType* = block:
        case cmakeBuildType.toLowerAscii():
        of "release":
            btRelease
        of "minsizerel":
            btMinSizeRel
        of "relwithdebinfo":
            btRelWithDebInfo
        of "debug":
            btDebug
        else:
            btCustom

    buildTypeName* = block:
        case buildType:
        of btDebug:
            "Debug"
        of btRelease:
            "Release"
        of btMinSizeRel:
            "MinSizeRel"
        of btRelWithDebInfo:
            "ReleaseWithDebInfo"
        of btCustom:
            cmakeBuildType
