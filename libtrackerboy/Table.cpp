
#include "trackerboy/Table.hpp"

#include <algorithm>

// IMPLEMENTATION DETAILS
//
// The Table class was rewritten without using std::map. It stores item data
// in a vector and keeps track of mapping by using a fixed array of 256 bytes.
// The fixed array, mMap, maps a table index to a vector index. If a table
// index does not have an associated vector index, or in other words, there
// isn't an item set to this id, it will map to 0xFF (or MAX_SIZE - 1).
//
// Existense of a mapping can be checked by testing if the vector index is
// less than the size of the vector (index < mItemData.size()). By default,
// the table is constructed with no mappings, or all table ids mapping to 0xFF
//
// Lookups are done like this: mItemData[mMap[id]]
//
// Items are added to the table by using the insert methods. An item can be
// inserted at the next available id or by specifying an id manually. A
// runtime_error will be thrown if the table is full (size of the vector is
// equal to MAX_SIZE) or a mapping already exists for the given id.
//
// Items can be removed by calling remove() with the desired id to remove.
// Removing an item will require updating the map array.
//
// Example remove
//
// Item Vector: (1, 3, 2, 4) // these are Ids
// Map: (all other Ids map to 0xFF)
// 1 -> 0
// 2 -> 2
// 3 -> 1
// 4 -> 3
//
// Remove item with id 1 (call remove(1))
//
// Item Vector: (3, 2, 4)
// Map:
// 1 -> 0xFF   // "remove" the mapping for #1
// 2 -> 1
// 3 -> 0
// 4 -> 2
//
// Basically all mappings that were ahead of id 1 in the vector have their
// vector indices subtracted by 1, to reflect the change by removing item #1
// from the vector
//
// Notes on item ownership:
// Items set in the table are owned by the table and will be destroyed with
// the table.
//
// Reasoning for this rewrite: to reduce memory overhead and to speed up
// lookups, as well as improving the public interface
//

namespace trackerboy {


template <class T>
Table<T>::TableItem::TableItem(uint8_t id, std::string name, T value) :
    id(id),
    name(name),
    value(value) 
{        
}


template <class T>
Table<T>::Table() :
    mItemData(),
    mNextId(0) 
{
    mMap.fill(MAX_SIZE - 1);
}

template <class T>
Table<T>::~Table() {

}

template <class T>
T* Table<T>::operator[](uint8_t id) {
    uint8_t index = mMap[id];
    if (index > mItemData.size()) {
        return nullptr;
    } else {
        return &mItemData[index].value;
    }
}

template <class T>
typename Table<T>::Iterator Table<T>::begin() {
    return mItemData.begin();
}

template <class T>
void Table<T>::clear() {
    mItemData.clear();
    mMap.fill(MAX_SIZE - 1);
    mNextId = 0;
}

template <class T>
typename Table<T>::Iterator Table<T>::end() {
    return mItemData.end();
}

template <class T>
T& Table<T>::insert() {
    return insert(mNextId, "Untitled " + std::to_string(mNextId));
}

template <class T>
T& Table<T>::insert(uint8_t id, std::string name) {
    uint8_t *mapping = mMap.data() + id;
    size_t size = mItemData.size();

    if (*mapping < size) {
        throw std::runtime_error("cannot insert: item already exists");
    } else if (size == MAX_SIZE) {
        throw std::runtime_error("cannot insert: table is full");
    }

    mItemData.emplace_back(id, name, T());
    uint8_t itemIndex = static_cast<uint8_t>(size);
    TableItem &item = mItemData[itemIndex];
    //item.id = id;
    //item.name = name;

    *mapping = itemIndex;
    if (mNextId == id) {
        findNextId();
    }

    return item.value;
}

template <class T>
std::string Table<T>::name(uint8_t id) {
    uint8_t index = mMap[id];
    if (index > mItemData.size()) {
        throw std::runtime_error("cannot get name: item does not exist");
    } else {
        return mItemData[index].name;
    }
}

template <class T>
void Table<T>::setName(uint8_t id, std::string name) {
    uint8_t index = mMap[id];
    if (index > mItemData.size()) {
        throw std::runtime_error("cannot set name: item does not exist");
    }

    mItemData[index].name = name;
}

template <class T>
size_t Table<T>::size() {
    return mItemData.size();
}


template <class T>
void Table<T>::remove(uint8_t id) {
    uint8_t index = mMap[id];
    if (index > mItemData.size()) {
        throw std::runtime_error("cannot remove: item does not exist");
    }

    // Note: this implementation slows down removals (worst case O(n)). But
    // since remove is expected to be called infrequently, this should not be
    // an issue

    // remove the mapping
    mMap[id] = MAX_SIZE - 1;

    // remove the item
    mItemData.erase(mItemData.begin() + index);
    // now adjust the mappings for all items ahead of the one we erased
    for (uint8_t i = index; i != mItemData.size(); ++i) {
        mMap[mItemData[i].id] = i;
    }

    if (mNextId > id) {
        mNextId = id; // always use the lowest available id first
    }
}

template <class T>
void Table<T>::findNextId() {
    size_t size = mItemData.size();
    if (size < MAX_SIZE) {
        // find the next available id
        while (mMap[++mNextId] < size);
    }
}


// we will only use the following tables:

template class Table<Instrument>;
template class Table<Song>;
template class Table<Waveform>;

}
