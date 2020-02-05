
#pragma once

#include <cstdint>
#include <string>

namespace trackerboy {

#pragma pack(push, 1)

struct Version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;

    std::string toString();
};

#pragma pack(pop)

extern const Version VERSION;

bool operator==(const Version &lhs, const Version &rhs);

bool operator<(const Version &lhs, const Version &rhs);



}
