# Source Code Organization

Individual components of this repo is organized into projects.
Each project has a CMakeLists.txt, private header files and source files, and
a folder for unit test code, `test`. 

Directories
 * `audio/`: audio project, mostly utility code for portaudio as well as filters
 * `demo/`: Source code for the demo project, Contains programs demonstrating the use of the library (portaudio is required)
 * `include/`: public header files for trackerboy and other libraries go here
 * `libtrackerboy/`: Source code for the trackerboy library project
 * `portaudio/`: Git submodule for portaudio repository, used by `demo` and `ui`
 * `ui/`: Source code for the ui project. Contains the main user interface for trackerboy


## Build system

When adding new source files you must add the file path to the corresponding
CMakeLists.txt. Globs are not to be used anywhere (explicit > implicit). Keep
the source list in alphabetical order when adding your file.

## Unit testing

Unit test code follows this naming scheme: `test_<module>.cpp`. Where
module is the name of the unit you are testing. This file resides in the
same directory as the module.

Catch2 is used as the unit testing framework.
