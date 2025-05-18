# Utility build tasks
switch "hints", "off"

var
  buildType = "Debug"
  buildDir = "build"
  configureArgs = ""

when fileExists("devconfig.nims"):
  include "devconfig.nims"

import std/strformat

let
  BuildCmd = &"cmake --build {buildDir}"

task configure, "CMake Configure":
  exec &"cmake -S . -B {buildDir} -DCMAKE_BUILD_TYPE={buildType} {configureArgs}"

task build, "Build all":
  exec BuildCmd

task run, "Run the frontend":
  exec BuildCmd & " --target frontend"
  exec &"{buildDir}/src/frontend2/trackerboy"

task testBackendLink, "Tests a sample C++ application linking with the backend":
  exec BuildCmd & " --target backendLinkTest"
  exec &"{buildDir}/src/backend/backendLinkTest"

task testBackend, "Tests the backend":
  exec BuildCmd & " --target buildBackendTest"
  exec &"{buildDir}/src/backend/backendTest"

task test, "Test all":
  testBackendLinkTask()
  testBackendTask()

