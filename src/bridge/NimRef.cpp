
#include "NimRef.hpp"
#include <utility>

namespace bridge {

template <class T>
NimRef<T>::NimRef() :
    mRef(nullptr)
{
}

template <class T>
NimRef<T>::NimRef(T *src) :
    mRef(src)
{
    if (src != nullptr) {
        GcRef(src);
    }
}

template <class T>
NimRef<T>::NimRef(NimRef<T> const& lhs)
{
    operator=(lhs);
}

template <class T>
NimRef<T>::NimRef(NimRef<T> &&lhs)
{
    operator=(std::move(lhs));
}

template <class T>
NimRef<T>::~NimRef() {
    if (mRef != nullptr) {
        GcUnref(mRef);
    }
}

template <class T>
NimRef<T>& NimRef<T>::operator=(NimRef<T> const& lhs) {
    mRef = lhs.mRef;
    if (mRef != nullptr) {
        GcRef(mRef);
    }
    return *this;
}

template <class T>
NimRef<T>& NimRef<T>::operator=(NimRef<T> &&lhs) {
    mRef = lhs.mRef;
    lhs.mRef = nullptr;
    return *this;
}

template <class T>
T* NimRef<T>::get() const noexcept {
    return mRef;
}

}
