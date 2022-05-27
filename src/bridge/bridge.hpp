/*
* Header for the C++ to Nim bridge. The bridge allows for
* C++ code to call Nim code. The main purpose of the bridge is
* to provide a C++ interface for libtrackerboy, which is
* written in Nim.
* 
*        +------->-------------<------+
* C Side | C++   <   bridge    >  Nim | Nim side
*        |       >  C++, Nim   <      |
*        +-------<------------->------+
*
*/

#pragma once

#include "NimRef.hpp"


namespace bridge {

// no specific convention
#define BRIDGE_API(ret, func) ret func

//
// Intialize Nim, must call once before calling any bridge function!
//
BRIDGE_API(void, NimMain)();

// Nim exception, catch on C side using catch (bridge::Exception *ex)
struct Exception;

// Gets the name of the given exception. The string returned is
// valid for the lifetime of ex.
BRIDGE_API(const char*, getExceptionName)(Exception* ex);

// Gets the message from the exception. The string returned is
// valid for the lifetime of ex.
BRIDGE_API(const char*, getExceptionMsg)(Exception* ex);

#ifdef NDEBUG
constexpr void printStackTrace(Exception* ex) { (void)ex; }
#else
// Debug builds only, prints the stack trace of the given exception
// to the debugger console on windows, or stdout everywhere else.
BRIDGE_API(void, printStackTrace)(Exception* ex);
#endif


// trackerboy/version.nim

struct Version {
    int major;
    int minor;
    int patch;
};

BRIDGE_API(Version, getAppVersion)();
BRIDGE_API(Version, getLibVersion)();

struct Module;
struct Song;


BRIDGE_API(NimRef<Module>, newModule)();


}

#undef BRIDGE_API
