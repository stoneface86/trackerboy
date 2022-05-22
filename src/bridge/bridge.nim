
import trackerboy

type
    TbVersion {.exportc.} = object
        major: cint
        minor: cint
        patch: cint

proc tb_getAppVersion(): TbVersion {.exportc, noconv.} =
    TbVersion(
        major: appVersion.major.cint,
        minor: appVersion.minor.cint,
        patch: appVersion.patch.cint
    )
