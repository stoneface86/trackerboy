
import utils
import ../trackerboy/platforms

const (rtmidiApi, vendorConfig*) = block:
    var res: tuple[rtmidiApi: string, vendorConfig: CompilerConfig]
    when @pWindows:
        res.rtmidiApi = "__WINDOWS_MM__"
        res.vendorConfig.passl("winmm.lib")
    elif @pMac:
        res.rtmidiApi = "__MACOSX_CORE__"
        res.vendorConfig.passl(
            # needed by rtmidi
            "-framework CoreMIDI",
            "-framework CoreAudio",
            "-framework CoreFoundation",
            # needed by miniaudio
            "-lm",
            "-lpthread"
        )
    elif @pLinux:
        res.rtmidiApi = "__LINUX_ALSA__"
        res.vendorConfig.passl("-lasound", "-lpthread")
    else:
        {. error: "Cannot vendor dependencies for this platform" .}
    res

const vendorPrefix = "../../vendored/"

template `%`(src: string): string =
    vendorPrefix & src


# miniaudio

# we set the C++ standard to C++17 via passC, but we don't want this standard
# for miniaudio, a C library.
const miniaudioCStd = block:
    when @pWindows:
        "/std:c11 "
    else:
        "-std=c11 "

# use the MA_NO_* definitions to exclude parts of miniaudio we don't need
{. compile(
    %"miniaudio/miniaudio.c", 
    miniaudioCStd &
    cdefines(
        "MA_NO_DECODING",
        "MA_NO_ENCODING",
        "MA_NO_GENERATION",
        "MA_NO_CUSTOM",
        "MA_NO_RESOURCE_MANAGER",
        "MA_NO_NODE_GRAPH",
        "MA_NO_ENGINE"
    )
) .}

# rtmidi

{. compile(
    %"rtmidi/RtMidi.cpp",
    cdefines(rtmidiApi)
) .}
