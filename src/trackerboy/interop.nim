# C/C++ interop utilities

type
    ccstringImpl {.importc: "const char*".} = cstring
    ccstring* = distinct ccstringImpl
        # distinct version of cstring that will be compiled as `const char*`
        # it behaves the same as cstring, except that immutability is forced.

{. push borrow .}

func `==`*(x, y: ccstring): bool
proc add*(x: var string, y: ccstring)
func high*(x: ccstring): int
func isNil*(x: ccstring): bool
func len*(x: ccstring): int
func low*(x: ccstring): int
func `$`*(x: ccstring): string

{. pop .}

converter toCCString*(str: string): ccstring =
    str.cstring.ccstring

template `[]`*(x: ccstring, i: int): char =
    cast[cstring](x)[i]
