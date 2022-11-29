# Source Code Organization 

Directories
 * `build/`: build directory, contents are not tracked by git
 * `cmake/`: CMake modules and utilities
 * `src/`: Source code
 * `test/`: Unit testing code
 * `vendored/`: Third-party code/libraries


## Build system

When adding new source files you must add the file path to the corresponding
CMakeLists.txt. Globs are not to be used anywhere (explicit > implicit). Keep
the source list in alphabetical order when adding your file.

## Unit testing

Qt Test is used as the unit testing framework. Unit test code resides in the
`test/` folder.
