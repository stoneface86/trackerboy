# platforms.nim
#
# Contains an enum for all OSes that Trackerboy supports. Convenience utility
# instead of doing `when defined(...)`
#

type
    Platform* = enum
        pWindows
        pLinux
        pMac
        pUnknown

const platform* = block:
    when defined(linux):
        pLinux
    elif defined(macosx):
        pMac
    elif defined(windows):
        pWindows
    else:
        pUnknown

template `@`*(p: static[Platform]): bool =
    platform == p
