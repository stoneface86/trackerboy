
#pragma once

namespace bridge {

//
// C++ Wrapper for a Nim-managed ref type. The managed ref will be
// unmarked on destruction.
// 
// The nim side of the bridge instantiates all possible ref types
//
template <class T>
class NimRef {
public:
    NimRef();
    explicit NimRef(T *r);
    NimRef(NimRef<T> const& lhs);
    NimRef(NimRef<T> &&lhs);
    ~NimRef();

    NimRef<T>& operator=(NimRef<T> const& lhs);
    NimRef<T>& operator=(NimRef<T> &&lhs);

    T* get() const noexcept;

private:
    T *mRef;

};



}
