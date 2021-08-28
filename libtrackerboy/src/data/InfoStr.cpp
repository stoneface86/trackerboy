#include "trackerboy/data/InfoStr.hpp"

#include <algorithm>
#include <cstring>

namespace trackerboy {

InfoStr::InfoStr(std::string const& str) noexcept {
    operator=(str);
}

InfoStr::InfoStr(const char *str) noexcept {
    auto len = std::min(strlen(str), size());
    std::copy_n(str, len, data());
}

InfoStr& InfoStr::operator=(std::string const& str) noexcept {
    std::copy_n(str.cbegin(), std::min(size(), str.size()), begin());
    return *this;
}

void InfoStr::clear() noexcept {
    fill('\0');
}

std::string InfoStr::toString() const noexcept {
    return { data(), size() };
}

}
