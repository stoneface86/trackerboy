
project(test LANGUAGES CXX)

enable_testing()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt5Test REQUIRED)

# each test in this list must have a cpp and hpp file in the units/ directory
# IMPORTANT: your test class must have a constructor taking no arguments and is marked with Q_INVOKABLE
set(TESTLIST
    "TestPatternClip"
    "TestPatternSelection"
)

set(TEST_SRC "")
set(INCLUDE_LIST "")
foreach (test IN ITEMS ${TESTLIST})
    list(APPEND TEST_SRC "units/${test}.cpp" "units/${test}.hpp")
    list(APPEND INCLUDE_LIST "#include \"units/${test}.hpp\"")
endforeach ()

string(REPLACE ";" "," CONFIG_TESTS "${TESTLIST}")
string(REPLACE ";" "\n" CONFIG_INCLUDES "${INCLUDE_LIST}")
configure_file("config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config.hpp" @ONLY)

# create one big executable
add_executable(test_trackerboy "main.cpp" "${TEST_SRC}" $<TARGET_OBJECTS:ui>)
target_include_directories(test_trackerboy PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(test_trackerboy PRIVATE ui Qt5::Test)

foreach (test IN ITEMS ${TESTLIST})
    add_test(NAME "${test}" COMMAND test_trackerboy "${test}")
endforeach ()


