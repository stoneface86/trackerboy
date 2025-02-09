
#pragma once

#include <cstddef>

//
// Namespace for accessing standard samplerates available for use in the
// application.
//
namespace StandardRates {

constexpr size_t COUNT = 7;

enum Rates {
    Rate11025 = 0,
    Rate12000 = 1,
    Rate22050 = 2,
    Rate24000 = 3,
    Rate44100 = 4,
    Rate48000 = 5,
    Rate96000 = 6
};

//
// Gets a standard samplerate from the given index (see Rates enum for possible choices).
//
int get(size_t index) noexcept;

}
