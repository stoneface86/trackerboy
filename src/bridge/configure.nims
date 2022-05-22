
import std/[os, strformat]

# this script just installs libtrackerboy locally if not done so already
# we will no longer need this script once libtrackerboy is published

const tag = "0809e678c99dde500db3e259fd0e908e022697e1"
const url = "https://github.com/stoneface86/libtrackerboy"
const clonePath = "libtrackerboy"


proc getHead(): string =
    if dirExists(clonePath):
        let gitDir = getCurrentDir().joinPath(clonePath).joinPath(".git")
        let cmdResult = gorgeEx(&"git --git-dir={gitDir} rev-parse HEAD")
        if cmdResult.exitCode == 0:
            result = cmdResult.output
            if result.len == 41:
                result.setLen(40)

proc checkout() =
    withDir(clonePath):
        exec &"git checkout {tag}"

let head = getHead()
if head.len == 0:
    rmDir(clonePath)
    exec &"git clone {url} {clonePath}"
    checkout()
elif head != tag:
    checkout()

let
    nimble = paramStr(3).quoteShell
    nimbleDir = paramStr(4).quoteShell
    nimbleCommand = &"{nimble} --nimbleDir:{nimbleDir}"

withDir(clonePath):
    exec &"{nimbleCommand} refresh"
    exec &"{nimbleCommand} -n install"

