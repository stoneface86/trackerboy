
find_path(Soundio_INCLUDE_DIR NAMES soundio/soundio.h)

find_library(Soundio_LIBRARY NAMES soundio)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Soundio DEFAULT_MSG Soundio_LIBRARY Soundio_INCLUDE_DIR)

mark_as_advanced(Soundio_INCLUDE_DIR Soundio_LIBRARY)
