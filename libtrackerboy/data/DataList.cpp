
#include "trackerboy/data/DataList.hpp"

#include <algorithm>

namespace trackerboy {

DataListBase::DataListBase() noexcept :
    mData(),
    mItemCount(0),
    mNextId(0u)
{
}

DataListBase::~DataListBase() noexcept {

}

void DataListBase::clear() noexcept {
    mData.clear();
    mItemCount = 0;
    mNextId = 0u;
}

size_t DataListBase::size() const noexcept {
    return mItemCount;
}

size_t DataListBase::capacity() const noexcept {
    return mData.size();
}

uint8_t DataListBase::nextAvailableId() const noexcept {
    return mNextId;
}

DataItem* DataListBase::insertItem() {
    return insertItem(mNextId);
}

DataItem* DataListBase::insertItem(uint8_t id) {
    if (mItemCount == MAX_SIZE) {
        throw std::runtime_error("cannot insert: maximum capacity reached");
    }

    if (id >= mData.size()) {
        mData.resize((size_t)id + 1);
    }

    auto &cell = mData[id];
    if (cell) {
        throw std::runtime_error("cannot insert: item already exists");
    }

    auto item = createItem();
    item->setId(id);
    cell.reset(item);
    ++mItemCount;

    findNextId();

    return item;
}

DataItem* DataListBase::duplicateItem(uint8_t id) {
    if (mItemCount == MAX_SIZE) {
        throw std::runtime_error("cannot duplicate: maximum capacity reached");
    }

    if (id < mData.size()) {
        auto &cellToDup = mData[id];
        if (cellToDup) {
            auto &cell = mData[mNextId];
            // copy constructor
            auto item = copyItem(*cellToDup);
            item->setId(mNextId);
            cell.reset(item);
            ++mItemCount;
            findNextId();
            return item;
        }
    }

    // id was either out of bounds of the data vector or the pointer was null
    throw std::runtime_error("cannot duplicate: item does not exist");
}

void DataListBase::remove(uint8_t id) {
    if (mItemCount == 0) {
        throw std::runtime_error("cannot remove: list is empty");
    }

    if (id < mData.size()) {
        auto &cell = mData[id];
        if (cell) {
            --mItemCount;
            cell.reset();
            if (mNextId > id) {
                mNextId = id;
            }
            return;
        }
    }

    throw std::runtime_error("cannot remove: item does not exist");
}

DataItem* DataListBase::itemAt(uint8_t id) {
    if (id >= mData.size()) {
        return nullptr;
    } else {
        return mData[id].get();
    }
}

void DataListBase::findNextId() {
    if (mItemCount < MAX_SIZE) {
        // find the next available id
        auto iter = mData.cbegin() + mNextId + 1;
        auto endIter = mData.cend();
        for (; iter < endIter; ++iter) {
            ++mNextId;
            if (!*iter) {
                return;
            }
        }
        mNextId = (uint8_t)mData.size();
    }
}

template <class T>
DataList<T>::DataList() :
    DataListBase()
{
}

template <class T>
DataList<T>::~DataList() {

}

template <class T>
T* DataList<T>::operator[](uint8_t id) {
    return static_cast<T*>(itemAt(id));
}

template <class T>
T* DataList<T>::insert() {
    return static_cast<T*>(insertItem());
}

template <class T>
T* DataList<T>::insert(uint8_t id) {
    return static_cast<T*>(insertItem(id));
}

template <class T>
T* DataList<T>::duplicate(uint8_t id) {
    return static_cast<T*>(duplicateItem(id));
}

template <class T>
DataItem* DataList<T>::createItem() {
    return new T();
}

template <class T>
DataItem* DataList<T>::copyItem(DataItem const& item) {
    return new T(static_cast<T const&>(item));
}

template class DataList<Instrument>;
template class DataList<Waveform>;

}
