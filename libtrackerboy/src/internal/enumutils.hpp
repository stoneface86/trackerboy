
#pragma once

#include <type_traits>

namespace trackerboy {

// convert an enum class to its underlying type using unary operator +
// so instead of
//      static_cast<std::underlying_type<Foo>>(Foo::bar)
// you can do this instead:
//      +Foo::bar
// 
template <typename T>
constexpr auto operator+(T e) noexcept
-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>>(e);
}

}
