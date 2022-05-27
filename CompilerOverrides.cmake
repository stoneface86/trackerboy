
if (MSVC)
    # remove default /W3 compiler flag
    string (REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")

    # remove default runtime library flag
    foreach (lang C CXX)
        string(REGEX REPLACE "/M[TD][d]?" "" CMAKE_${lang}_FLAGS_INIT "${CMAKE_${lang}_FLAGS_INIT}")
        foreach (config DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
            set(var CMAKE_${lang}_FLAGS_${config}_INIT)
            string(REGEX REPLACE "/M[TD][d]?" "" CMAKE_${lang}_FLAGS_${config}_INIT "${CMAKE_${lang}_FLAGS_${config}_INIT}")
            unset(var)
        endforeach ()
    endforeach ()
endif ()
