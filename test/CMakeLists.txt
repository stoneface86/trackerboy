
project(test LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOMOC ON)
find_package(Qt5Test REQUIRED)

set(TESTLIST
    "TestPatternClip"
)

foreach (test "${TESTLIST}")

    add_executable("${test}" "${test}.cpp")
    target_link_libraries("${test}" PRIVATE ui Qt5::Test)
    add_test("${test}" "${test}")

endforeach ()

enable_testing()

