
import cmake

# set switches based on CMAKE_BUILD_TYPE
case buildType:
of btDebug:
    discard
of btRelease:
    switch("define", "release")
of btMinSizeRel:
    switch("define", "release")
    switch("opt", "size")
of btRelWithDebInfo:
    switch("define", "release")
    switch("debuginfo", "on")
of btCustom:
    discard

# add other switches here if needed
# ie:
# switch("mm", "orc")
# switch("threads", "on")
