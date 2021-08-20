#pragma once

#include <array>
#include <string>

namespace trackerboy {

//
// 32 byte character information string used in the module header
//
class InfoStr : public std::array<char, 32> {

public:
    InfoStr() = default;
    // implicit constructor from the given string
    InfoStr(std::string const& str) noexcept;

    InfoStr(const char *str) noexcept;

    InfoStr& operator=(std::string const& str) noexcept;

    void clear() noexcept;

    std::string toString() const noexcept;

};

}
