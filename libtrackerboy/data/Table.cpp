
#include "trackerboy/data/Table.hpp"

#include <algorithm>
#include <stdexcept>

namespace trackerboy {

/*
// Implementation details
// 
// Items are dynamically allocated when inserted or duplicated. shared_ptr is used
// so that the engine can continue to use items after they have been removed from the
// table. The shared_ptrs are stored in a fixed size array, mData. This way lookup is
// done by accessing the array by the item's id.
//
// The thin-template idiom is used, BaseTable does most of the work. The Table<T> class
// simply downcasts the result from BaseTable to the templated type.
*/



BaseTable::BaseTable() noexcept :
    mData(),
    mSize(0u),
    mNextId(0u)
{
}

BaseTable::~BaseTable() noexcept {

}

void BaseTable::clear() noexcept {
    mData.fill(nullptr);
    mSize = 0u;
    mNextId = 0u;
}

size_t BaseTable::size() const noexcept {
    return mSize;
}

uint8_t BaseTable::nextAvailableId() const noexcept {
    return mNextId;
}

DataItem& BaseTable::insert() {
    return insert(mNextId);
}

DataItem& BaseTable::insert(uint8_t id) {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot insert: maximum capacity reached");
    }

    auto &cell = mData[id];
    if (cell) {
        throw std::runtime_error("cannot insert: item already exists");
    }

    auto item = createItem();
    item->setId(id);
    cell = std::move(item);
    ++mSize;
    if (mNextId == id) {
        findNextId();
    }

    return *cell.get();
}

DataItem& BaseTable::duplicate(uint8_t id) {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot duplicate: maximum capacity reached");
    }

    auto &toDup = mData[id];
    if (toDup) {
        auto item = copyItem(*toDup);
        auto &result = *item.get();
        item->setId(mNextId);
        mData[mNextId] = std::move(item);
        ++mSize;
        findNextId();
        return result;
    } else {
        throw std::runtime_error("cannot duplicate: item does not exist");
    }
    
}

void BaseTable::remove(uint8_t id) {
    if (size() == 0) {
        throw std::runtime_error("cannot remove: list is empty");
    }

    if (id < mData.size()) {
        auto &cell = mData[id];
        if (cell) {
            --mSize;
            cell.reset();
            if (mNextId > id) {
                mNextId = id;
            }
            return;
        }
    }

    throw std::runtime_error("cannot remove: item does not exist");
}

DataItem const* BaseTable::get(uint8_t id) const {
    if (id >= mData.size()) {
        return nullptr;
    } else {
        return mData[id].get();
    }
}

DataItem* BaseTable::get(uint8_t id) {
    return const_cast<DataItem*>(
        const_cast<BaseTable const*>(this)->get(id)
        );
}

std::shared_ptr<DataItem> BaseTable::getShared(uint8_t id) const {
    if (id >= mData.size()) {
        return nullptr;
    } else {
        return mData[id];
    }
}

void BaseTable::findNextId() {
    if (size() < MAX_SIZE) {
        // find the next available id
        do {
            ++mNextId;
        } while (mData[mNextId]);
    }
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
T const* Table<T>::operator[](uint8_t id) const {
    return get(id);
}

template <class T>
T* Table<T>::operator[](uint8_t id) {
    return get(id);
}

template <class T>
T& Table<T>::insert() {
    return static_cast<T&>(BaseTable::insert());
}

template <class T>
T& Table<T>::insert(uint8_t id) {
    return static_cast<T&>(BaseTable::insert(id));
}

template <class T>
T& Table<T>::duplicate(uint8_t id) {
    return static_cast<T&>(BaseTable::duplicate(id));
}

template <class T>
T* Table<T>::get(uint8_t id) {
    return static_cast<T*>(BaseTable::get(id));
}

template <class T>
T const* Table<T>::get(uint8_t id) const {
    return static_cast<T const*>(BaseTable::get(id));
}

template <class T>
std::shared_ptr<T> Table<T>::getShared(uint8_t id) const {
    return std::static_pointer_cast<T>(BaseTable::getShared(id));
}

template <class T>
std::shared_ptr<DataItem> Table<T>::createItem() {
    return std::make_shared<T>();
}

template <class T>
std::shared_ptr<DataItem> Table<T>::copyItem(DataItem const& item) {
    return std::make_shared<T>(static_cast<T const&>(item));
}

template class Table<Instrument>;
template class Table<Waveform>;






}
