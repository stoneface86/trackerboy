#include "trackerboy/data/InfoStr.hpp"

#include <algorithm>
#include <cstring>

namespace trackerboy {

InfoStr::InfoStr(std::string const& str) noexcept {
    operator=(str);
}

InfoStr::InfoStr(const char *str) noexcept {
    operator=(str);
}

InfoStr& InfoStr::operator=(std::string const& str) noexcept {
    auto _size = std::min(size(), str.size());
    std::copy_n(str.cbegin(), _size, begin());
    std::fill(begin() + _size, end(), '\0');
    return *this;
}

InfoStr& InfoStr::operator=(const char *str) noexcept {
    auto len = std::min(strlen(str), size());
    std::copy_n(str, len, data());
    std::fill(begin() + len, end(), '\0');
    return *this;
}

void InfoStr::clear() noexcept {
    fill('\0');
}

std::string InfoStr::toString() const noexcept {
    return { data(), size() };
}

size_t InfoStr::length() const noexcept {
    if (*rbegin() != '\0') {
        return size();
    } else {
        return strlen(data());
    }
}

}
