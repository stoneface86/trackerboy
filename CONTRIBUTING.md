# Contributing Guidelines

Any contribution is welcome at this point in time. For work check the issues
or the project page. If you are still unsure what to do, please contact me,
[stoneface86](https://github.com/stoneface86)

## Bugs

If you have found a bug or problem, please create an
[issue](https://github.com/stoneface86/trackerboy/issues) for it. When
posting the issue, be sure to include as much information about the bug
as possible such as screenshots, log dumps, module files, etc. At the
very least, your issue should describe how to reproduce the bug.

You can also submit a pull request that fixes the bug, just make sure you
follow the same guidelines for pull requests.

## Pull Requests

If you are interested in Trackerboy's development, pull requests are a good way to
start. I would prefer it if you contact me before you start any work, as I
have final say in what gets merged. If I don't like it, it doesn't get merged.
You can contact me on [Twitter](https://twitter.com/stoneface86) or
[Discord](https://discord.gg/m6wcAK3). Best way to reach me is by pinging me
in the discord server.

Always base your branch on the develop branch. The master branch points to the
latest release, so you'll want your changes to be based off the latest in-development
changes.

Your code should also compile without warnings.

For coding style, please follow the guidelines I have listed below and look at
existing code for examples. I'm not strict with style, but be reasonable. If your
code looks out of place compared to everything else I will ask you to revise it.

The rest of this document defines how you should write your code.

See [ORGANIZATION.md](ORGANIZATION.md) for details on where to put source files,
as well as editing the build system.

### Style guidelines

All code in this repo should be consistent in style and naming scheme. Please
follow these rules when contributing:
 * 4-space indentation, use spaces
 * Unix style newlines (LF) for all files
 * curly braces on same line, except for constructors
 * class/struct/typedef names should use PascalCase
 * function names and variables should use camelCase
 * forward-declare in headers when possible
 * member variables must be prefixed with m (ie mFooBar) (except for structs)
 * try to avoid macros in headers, use constexpr functions when possible and
   use enums or constexpr instead of `#define`. Rely on the compiler, not the preprocessor
 * avoid `if(cond) statement;` always use braces.
 * summarize in comments what your code does (but don't comment every line)
 * C-style casts should not be used except for integer conversions
   (ie int -> uint8_t)

### Miniaudio

Miniaudio is a C header-only library that Trackerboy uses for audio playback.

Do not call any miniaudio functions or use miniaudio types/constants except
in these classes:
 * AudioEnumerator
 * AudioStream
 * RingbufferBase

This is to make a potential refactor painless if we ever need to switch audio
libs.

### RtMidi

RtMidi is a C/C++ library that Trackerboy uses for MIDI input.

Similiarly as with miniaudio, do not interface with RtMidi except for the
following classes:
 * Midi
 * MidiEnumerator

### TU namespacing

In order to prevent name-clashes and ODR violations when unity building, use
the TU namespace in your source files for any static constants/functions etc.
Do not use anonymous namespaces! (Note: TU is short for "this unit").

Example:
```cpp
// TU is a shortcut macro
// the namespace will be named using the source unit's name suffixed with TU
#define TU MyClassTU
namespace TU {

    // constants, functions, etc
    
}

// undefine TU at the end of the source file
#undef TU

```

### Notes on integers

The use of unsigned integers seems to be a controversal topic in C++,
so I'm going to follow the advice of Bjarne Stroustrup, which is to avoid them.
Fixed-width and unsigned integers should be avoided, only to be used in certain
circumstances such as:
 1. bitwise operations / modulo arithmetic
 2. `size_t` for array indexing / or use with std library
    (except when possible to use int, see below)
 3. data serialization, or where the format of a struct is fixed
 4. memory conservation

`unsigned` should not be used for nonnegative integers, always use `int`.
Always default to using `int` unless you have a good reason otherwise.

When indexing an array, prefer to use `int` unless you need the full range of
`size_t`. The sizes of all arrays/vectors used in this project are well within
the limits of `int`, so there should be rarely any need to use `size_t`.

Note: there is still a lot of code in this repo that doesn't follow this rule,
as it is slowly being refactored. All new code however, should follow this rule.

### Documentation

Functions, methods, classes and typedefs should have a comment block before the
declaration stating its purpose. A simple sentence or two explaining what it is
and/or what it's for will suffice. Doxygen is not used for this project, but you
may use doxygen style formatting if you prefer.
