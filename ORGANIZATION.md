# Source Code Organization

Individual components of this repo is organized into projects.
Each project has a CMakeLists.txt, private header files and source files, and
a folder for unit test code, `test`. 

Directories
 * `demo/`: Source code for the demo project, Contains programs demonstrating the use of the library (portaudio is required)
 * `include/`: public header files for trackerboy and other libraries go here
 * `libtrackerboy/`: Source code for the trackerboy library project
 * `portaudio/`: Git submodule for portaudio repository, used by `demo` and `ui`
 * `ui/`: Source code for the ui project. Contains the main user interface for trackerboy
