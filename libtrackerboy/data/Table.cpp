
#include "trackerboy/data/Table.hpp"

#include "./checkedstream.hpp"

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

FormatError BaseTable::deserialize(std::istream &stream) noexcept {

    // get the size of the table
    uint16_t tableSize = 0;
    checkedRead(stream, &tableSize, sizeof(tableSize));
    tableSize = correctEndian(tableSize);

    if (tableSize > MAX_SIZE) {
        return FormatError::tableSizeBounds;
    }

    // clear just in case
    clear();

    for (uint16_t i = 0; i != tableSize; ++i) {
        DataItem *item = createItem();
        FormatError error = item->deserialize(stream);
        if (error == FormatError::none) {
            // item deserialize successfully
            auto id = item->id();
            auto &ptr = (*mData)[id];
            if (ptr) {
                // duplicate id!
                delete item;
                return FormatError::tableDuplicateId;
            }

            ptr.reset(item);
            // no need to sort, serializing writes items in order
            mItemOrder.push_back(id);
        } else {
            // read error occurred, delete item and return
            delete item;
            return error;
        }
    }

    // if there was an item with id = 0, find the next available id
    if ((*mData)[0]) {
        findNextId();
    }

    return FormatError::none;
}

DataItem& BaseTable::duplicate(uint8_t id) {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot duplicate: table is full");
    }
    
    auto *item = get(id);
    if (item == nullptr) {
        throw std::runtime_error("cannot duplicate item: item does not exist");
    }

    auto &ptr = (*mData)[mNextId];
    if (ptr) {
        throw std::runtime_error("cannot insert: item already exists");
    }

    auto copy = copyItem(*item);
    copy->setId(mNextId);

    finishInsert(copy);

    return *copy;
    
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

uint8_t BaseTable::nextId() const noexcept {
    return mNextId;
}

int BaseTable::nextModelId() const noexcept {
    int index = 0;
    for (auto id : mItemOrder) {
        if (mNextId < id) {
            break;
        }
        ++index;
    }
    return index;
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

FormatError BaseTable::serialize(std::ostream &stream) noexcept {

    uint16_t count = correctEndian(static_cast<uint16_t>(size()));
    stream.write(reinterpret_cast<const char *>(&count), sizeof(count));
    if (!stream.good()) {
        return FormatError::writeError;
    }

    for (auto id : mItemOrder) {
        DataItem *item = get(id);
        FormatError error = item->serialize(stream);
        if (error != FormatError::none) {
            return error;
        }
    }

    return FormatError::none;
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

DataItem& BaseTable::insertItem() {
    return insertItem(mNextId, "Untitled " + std::to_string(mNextId));
}

DataItem& BaseTable::insertItem(uint8_t id, std::string name) {
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
    finishInsert(item);
    

    return *item;
}

void BaseTable::finishInsert(DataItem *item) {
    uint8_t id = item->id();
    (*mData)[id].reset(item);
    mItemOrder.insert(std::upper_bound(mItemOrder.begin(), mItemOrder.end(), id), id);

    if (mNextId == id) {
        findNextId();
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
T* Table<T>::operator[](uint8_t id) {
    return static_cast<T*>(get(id));
}

template <class T>
T& Table<T>::insert() {
    return static_cast<T&>(insertItem());
}

template <class T>
T& Table<T>::insert(uint8_t id, std::string name) {
    return static_cast<T&>(insertItem(id, name));
}

template <class T>
DataItem* Table<T>::createItem() {
    return new T();
}

template <class T>
DataItem* Table<T>::copyItem(DataItem &item) {
    return new T(static_cast<T&>(item));
}


// we will only use the following tables:

template class Table<Instrument>;
template class Table<Waveform>;

}
