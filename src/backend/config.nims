switch "app",       "staticLib"
switch "noMain",    "on"
switch "backend",   "c"
switch "p",         "@CMAKE_CURRENT_SOURCE_DIR@"
switch "cc",        "@NIM_CC@"
switch "nimcache",  "@CMAKE_CURRENT_BINARY_DIR@/nimcache"
switch "out",       "@BACKEND_LIB_PATH@"
switch "define",    "noSignalHandler"

case "@CMAKE_BUILD_TYPE@"
of "RelWithDebInfo":
  switch "define", "release"
  switch "debuginfo", "on"
of "MinSizeRel":
  switch "define", "release"
  switch "opt", "size"
of "Release":
  switch "define", "release"
else:
  switch "debuginfo", "on"