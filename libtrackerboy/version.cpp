
#include "trackerboy/version.hpp"

#include <sstream>

#if !defined(VERSION_MAJOR) || !defined(VERSION_MINOR) || !defined(VERSION_PATCH)
#error version information not provided
#endif

namespace trackerboy {

const Version VERSION = { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH };

std::string Version::toString() {
    std::ostringstream out;
    out << major << "." << minor << "." << patch;
    return out.str();
}

bool operator==(const Version &lhs, const Version &rhs) {
    return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
}

bool operator<(const Version &lhs, const Version &rhs) {
    if (lhs.major == rhs.major) {
        if (lhs.minor == rhs.minor) {
            return lhs.patch < rhs.patch;
        } else {
            return lhs.minor < rhs.minor;
        }
    } else {
        return lhs.major < rhs.major;
    }
}


}
