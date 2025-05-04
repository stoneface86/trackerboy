switch "app",       "staticLib"
switch "noMain",    "on"
switch "backend",   "cpp"
switch "cppCompileToNamespace", "B"
switch "p",         "@CMAKE_CURRENT_SOURCE_DIR@"
switch "cc",        "@NIM_CC@"
switch "nimcache",  "@CMAKE_CURRENT_BINARY_DIR@/nimcache"
switch "out",       "@BACKEND_LIB_PATH@"
switch "define",    "noSignalHandler"
switch "header",    "tbb_nim"
@BACKEND_INCLUDES_CODE@

case "@CMAKE_BUILD_TYPE@"
of "RelWithDebInfo":
  switch "define", "release"
  switch "debuginfo", "on"
of "MinSizeRel":
  switch "define", "release"
  switch "opt", "size"
  switch "define", "strip"
of "Release":
  switch "define", "release"
else:
  switch "debuginfo", "on"