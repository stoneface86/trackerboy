
#include "trackerboy/Table.hpp"

namespace trackerboy {

template <class T>
Table<T>::Table() :
    nextId(0) 
{        
}

template <class T>
T* Table<T>::operator[](uint8_t id) {
    auto iter = mData.find(id);

    if (iter != mData.end()) {
        return &iter->second;
    } else {
        return nullptr;
    }
}

template <class T>
uint8_t Table<T>::add(T data) {
    if (mData.size() >= 256) {
        return 0; // TODO: throw exception here instead
    }

    mData[nextId] = data;

    uint8_t id = nextId;
    findNextId();

    return id;
}

template <class T>
void Table<T>::remove(uint8_t id) {
    mData.erase(id);
    if (nextId > id) {
        nextId = id; // always use the lowest available id first
    }
}

template <class T>
void Table<T>::findNextId() {
    if (mData.size() < 256) {
        // find the next available id
        while (mData.count(++nextId) != 0);
    }
}




template class Table<Instrument>;
template class Table<Song>;
template class Table<Waveform>;

}