
#pragma once

#include <type_traits>

//
// Connects an action's triggered signal to a slot for this
//
#define connectActionToThis(action, slot)                                      \
    connect(action, &QAction::triggered, this,                                 \
            &std::decay_t<decltype(*this)>::slot)

//
// Convenience macro for connecting signals. Doesn't work with overloads.
//  lazyconnect(foo, barChanged, biz, setBar) => connect(foo, &Foo::barChanged,
//  biz, &Biz::setBar)
//
#define lazyconnect(src, signal, dest, slot)                                   \
    connect(src, &std::decay_t<decltype(*src)>::signal, dest,                  \
            &std::decay_t<decltype(*dest)>::slot)

//
// Connects an action's triggered signal to the given slot
//
#define connectActionTo(action, dest, slot)                                    \
    connect(action, &QAction::triggered, dest,                                 \
            &std::decay_t<decltype(*dest)>::slot)

//
// Connects a signal to a lambda function
//
#define connectLambda(src, signal, dest, lambda)                               \
    connect(src, &std::decay_t<decltype(*src)>::signal, dest, lambda)

//
// Gets the method pointer of a slot or signal from the given object.
//
// Example:
//   connect(act, lazyslot(act, triggered), ...)
//   // becomes
//   connect(act, &QAction::triggered, ...)
//
#define lazyslot(obj, slot) &std::decay_t<decltype(*obj)>::slot

//
// Convenience macro returns a parameter pair of `obj` and `lazyslot(obj, slot)`
// ie, `connect(lazyslotx(act, triggered), ...)` becomes `connect(act,
// &QAction::triggered, ...)`
//
#define lazyslotx(obj, slot) obj, &std::decay_t<decltype(*obj)>::slot