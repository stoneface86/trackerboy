
#include "trackerboy/data/Table.hpp"

#include <algorithm>

// IMPLEMENTATION DETAILS
//
// The Table class stores DataItems in a 256 element array of unique_ptrs. Looking
// up based on item id is done by using the id as an index and getting the unique_ptr's
// pointer. The id's in use are stored in a sorted vector<uint8_t>. This vector is also
// used for the keeping track of the table's size.


namespace trackerboy {

BaseTable::BaseTable() noexcept :
    mData(new ArrayType()),
    mItemOrder(),
    mNextId(0)
{
}

BaseTable::~BaseTable() noexcept {

}

BaseTable::Iterator BaseTable::begin() const {
    return mItemOrder.cbegin();
}

BaseTable::Iterator BaseTable::end() const {
    return mItemOrder.cend();
}

DataItem* BaseTable::get(uint8_t id) const {
    return (*mData)[id].get();
}

DataItem* BaseTable::getFromOrder(uint8_t order) const {
    return (*mData)[mItemOrder[order]].get();
}

uint8_t BaseTable::lookup(uint8_t order) const {
    if (order >= mItemOrder.size()) {
        throw std::invalid_argument("cannot lookup item id: argument out of range");
    }

    return mItemOrder[order];
}

void BaseTable::remove(uint8_t id) {
    auto &ptr = (*mData)[id];
    if (!ptr) {
        throw std::runtime_error("cannot remove: item does not exist");
    }

    ptr.reset();
    mItemOrder.erase(std::remove(mItemOrder.begin(), mItemOrder.end(), id), mItemOrder.end());
    if (mNextId > id) {
        mNextId = id; // always use the lowest available id first
    }
}

size_t BaseTable::size() const noexcept {
    return mItemOrder.size();
}

void BaseTable::clear() noexcept {
    for (auto id : mItemOrder) {
        (*mData)[id].reset();
    }
    mItemOrder.clear();
    mNextId = 0;
}

void BaseTable::findNextId() noexcept {
    size_t size = mItemOrder.size();
    if (size < MAX_SIZE) {
        // find the next available id
        while ((*mData)[++mNextId]);
    }
}

DataItem& BaseTable::_insert() {
    return _insert(mNextId, "Untitled " + std::to_string(mNextId));
}

DataItem& BaseTable::_insert(uint8_t id, std::string name) {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot insert: table is full");
    }

    auto &ptr = (*mData)[id];
    if (ptr) {
        throw std::runtime_error("cannot insert: item already exists");
    }

    DataItem *item = createItem();
    item->setId(id);
    item->setName(name);
    ptr.reset(item);
    mItemOrder.insert(std::upper_bound(mItemOrder.begin(), mItemOrder.end(), id), id);

    if (mNextId == id) {
        findNextId();
    }

    return *item;
}


template <class T>
Table<T>::Table() :
    BaseTable()
{
}

template <class T>
Table<T>::~Table() {
}

template <class T>
T* Table<T>::operator[](uint8_t id) {
    return static_cast<T*>(get(id));
}

template <class T>
T& Table<T>::insert() {
    return static_cast<T&>(_insert());
}

template <class T>
T& Table<T>::insert(uint8_t id, std::string name) {
    return static_cast<T&>(_insert(id, name));
}

template <class T>
DataItem* Table<T>::createItem() {
    return new T();
}


// we will only use the following tables:

template class Table<Instrument>;
template class Table<Waveform>;

}
