
# build system utilities

import std/os

type
    CompilerConfig* = object
        ## Container object for compiler and linker flags to pass via
        ## the passC and passL pragmas.
        cflags*: seq[string]
        lflags*: seq[string]

const definesPrefix = block:
    when defined(vcc):
        "/D"
    else:
        "-D"

{. push compileTime .}

func cdefines*(defines: varargs[string]): string =
    if defines.len >= 1:
        for def in defines:
            result.add(definesPrefix)
            result.add(def)
            result.add(' ')
        result.setLen(result.len - 1)

proc passc*(c: var CompilerConfig, flags: varargs[string]) =
    c.cflags.add(flags)

proc passDefines*(c: var CompilerConfig, defines: varargs[string]) =
    for def in defines:
        c.cflags.add(definesPrefix & def)

proc passl*(c: var CompilerConfig, flags: varargs[string]) =
    c.lflags.add(flags)

func merge*(a, b: CompilerConfig): CompilerConfig =
    # Combines the two configs into one
    result.cflags.add(a.cflags)
    result.cflags.add(b.cflags)
    result.lflags.add(a.lflags)
    result.lflags.add(b.lflags)


template apply*(c: CompilerConfig) =
    mixin quoteShellCommand
    {. passC: c.cflags.quoteShellCommand() .}
    {. passL: c.lflags.quoteShellCommand() .}

{. pop .}
