
#pragma once

#include <QObject>

#include <optional>
#include <cstdint>

//
// Converts a Qt keycode to a hex digit if possible. std::nullopt is returned
// if the keycode is not a hex digit
//
std::optional<uint8_t> keyToHex(int const key);

//
// Replaces the upper or lower nibble in value with the given nibble
// ex:
// replaceNibble(0x32, 0xF, false) => 0x3F
// replaceNibble(0x32, 0xF, true)  => 0xF2
//
uint8_t replaceNibble(uint8_t value, uint8_t nibble, bool highNibble);

//
// Returns true if the given QObject's thread affinity is the same
// thread as the caller. If true is returned, then it is typically
// safe to modify the object without synchronization primitives.
//
bool objectInCurrentThread(QObject const& object);
