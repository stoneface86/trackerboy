# Contributing Guidelines

Any contribution is welcome at this point in time. For work check the issues
or the project page. If you are still unsure what to do, please contact me,
[stoneface86](https://github.com/stoneface86)

## Pull Request Process

1. Fork and create a new branch. The branch name should be related to what
   you are working on. Commit your changes to this branch and then squash all
   commits into 1 commit when finished.

2. Make sure everything compiles before submitting. If there are warnings
   please fix them. Also make sure your code follows the style guidelines.

3. Submit your pull request and I will review it. Be sure to explain your
   changes in the request. If all is good, I will merge the request. I may
   ask you to revise a couple things or I may outright deny the request.


## Style guidelines

All code in this repo should be consistent in style and naming scheme. Please
follow these rules when contributing:
 * 4-space indentation, use spaces
 * Unix style newlines (LF) for all files
 * curly braces on same line, except for constructors
 * class/struct/typedef names should use PascalCase
 * function names and variables should use camelCase
 * one class/type per header, one implementation per source file (except for
   inner classes and other tightly coupled types).
 * member variables must be prefixed with m (ie mFooBar) (except for structs)
 * try to avoid macros in headers, use constexpr functions when possible and
   use enums or constexpr instead of `#define`
 * avoid `if(cond) statement;` always use braces.
 * summarize in comments what your code does (don't comment every line)
 * C-style casts should not be used except for integer conversions
   (ie int -> uint8_t)

See [ORGANIZATION.md](ORGANIZATION.md) for details on where to put source files,
as well as editing the build system.

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

TBD
