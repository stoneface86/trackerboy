
#pragma once

#include <type_traits>

//
// Connects an action's triggered signal to a slot for this
//
#define connectActionToThis(action, slot) \
    connect(action, &QAction::triggered, this, &std::remove_pointer_t<decltype(this)>::slot)

//
// Convenience macro for connecting signals. Doesn't work with overloads.
//  lazyconnect(foo, barChanged, biz, setBar) => connect(foo, &Foo::barChanged, biz, &Biz::setBar)
//
#define lazyconnect(src, signal, dest, slot) \
    connect(src, &std::remove_pointer_t<decltype(src)>::signal, dest, &std::remove_pointer_t<decltype(dest)>::slot)

//
// Connects an action's triggered signal to the given slot
//
#define connectActionTo(action, dest, slot) \
    connect(action, &QAction::triggered, dest, &std::remove_pointer_t<decltype(dest)>::slot)
