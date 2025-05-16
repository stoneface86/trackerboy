
#pragma once

#include "tbb.hxx"

template <typename T>
struct NimRef {
    T * ref;

    NimRef() : ref(nullptr) { }
    NimRef(T *ref): ref(ref) { }
    NimRef(NimRef<T> const& x) : ref(x.ref) {
        B::ref(x.ref);
    }
    NimRef(NimRef<T> &&x) : ref(x.ref) {
        x.ref = nullptr;
    }
    ~NimRef() {
        if (ref) {
            B::unref(ref);
        }
    }
    NimRef<T>& operator=(NimRef<T> const& x) {
        this->ref = x.ref;
        B::ref(x.ref);
        return *this;
    }
    NimRef<T>& operator=(NimRef<T> &&x) {
        this->ref = x.ref;
        x.ref = nullptr;
        return *this;
    }

    operator bool() const {
        return ref != nullptr; 
    }

    T& operator*() const {
        return *ref;
    }

    T* operator->() const {
        return ref;
    }

};

template <typename T>
static constexpr NimRef<T> makeNimRef(T *ref) {
    return { ref };
}

