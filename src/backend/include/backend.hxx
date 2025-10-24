
#pragma once

#include "tbb.hxx"

///
/// Alias to signify that a pointer is actually a Nim reference type
///
template <class T>
using NimRef = T *;
