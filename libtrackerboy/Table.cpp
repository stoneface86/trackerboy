
#include "trackerboy/Table.hpp"

namespace trackerboy {

template <class T>
Table<T>::TableItem::TableItem() :
    renamed(false),
    name(""),
    value() 
{        
}


template <class T>
Table<T>::Table() :
    mUntitledCounter(0),
    mNextId(0) 
{        
}

template <class T>
T* Table<T>::operator[](uint8_t id) {
    auto iter = mData.find(id);

    if (iter != mData.end()) {
        return &iter->second.value;
    } else {
        return nullptr;
    }
}

template <class T>
uint8_t Table<T>::add(T &data) {
    if (mData.size() >= 256) {
        throw std::runtime_error("table is full");
    }

    uint8_t id = mNextId;
    set(id, data);
    return id;
}

template <class T>
std::string Table<T>::name(uint8_t id) {
    auto iter = mData.find(id);
    if (iter == mData.end()) {
        throw std::runtime_error("cannot get name: item does not exist");
    }
    return iter->second.name;
}

template <class T>
void Table<T>::set(uint8_t id, T &data) {
    
    bool isNew = mData.find(id) == mData.end();

    TableItem &item = mData[id];
    item.value = data;
    if (isNew) {
        item.name = "Untitled " + std::to_string(mUntitledCounter++);
        item.renamed = false;
    }

    if (mNextId == id) {
        findNextId();
    }
}

template <class T>
void Table<T>::setName(uint8_t id, std::string name) {
    if (mData.find(id) == mData.end()) {
        throw std::runtime_error("cannot set name: item does not exist");
    }

    TableItem &item = mData[id];
    item.name = name;
    if (!item.renamed) {
        mUntitledCounter--;
        item.renamed = true;
    }
}


template <class T>
void Table<T>::remove(uint8_t id) {
    auto iter = mData.find(id);
    if (iter != mData.end()) {
        if (!iter->second.renamed) {
            mUntitledCounter--;
        }
        mData.erase(id);
        if (mNextId > id) {
            mNextId = id; // always use the lowest available id first
        }
    }
}

template <class T>
void Table<T>::findNextId() {
    if (mData.size() < 256) {
        // find the next available id
        while (mData.count(++mNextId) != 0);
    }
}


// we will only use the following tables:

template class Table<Instrument>;
template class Table<Song>;
template class Table<Waveform>;

}