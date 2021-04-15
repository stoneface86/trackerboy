
#include "trackerboy/data/DataList.hpp"

#include <algorithm>
#include <stdexcept>

namespace trackerboy {

DataListBase::DataListBase() noexcept :
    mData(),
    mNextId(0u)
{
}

DataListBase::~DataListBase() noexcept {

}

DataListBase::Iterator DataListBase::begin() const noexcept {
    return mIdsInUse.cbegin();
}

void DataListBase::clear() noexcept {
    mData.clear();
    mIdsInUse.clear();
    mNextId = 0u;
}

DataListBase::Iterator DataListBase::end() const noexcept {
    return mIdsInUse.cend();
}

size_t DataListBase::size() const noexcept {
    return mIdsInUse.size();
}

size_t DataListBase::capacity() const noexcept {
    return mData.size();
}

uint8_t DataListBase::nextAvailableId() const noexcept {
    return mNextId;
}

DataItem* DataListBase::insert() {
    return insert(mNextId);
}

DataItem* DataListBase::insert(uint8_t id) {
    if (size() == MAX_SIZE) {
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
    cell = std::move(item);
    mIdsInUse.insert(id);
    findNextId();

    return cell.get();
}

DataItem* DataListBase::duplicate(uint8_t id) {
    if (size() == MAX_SIZE) {
        throw std::runtime_error("cannot duplicate: maximum capacity reached");
    }

    if (id < mData.size()) {
        if (mData[id]) {
            if (mNextId >= mData.size()) {
                mData.resize((size_t)mNextId + 1);
            }
            auto &cell = mData[mNextId];
            // copy constructor
            auto item = copyItem(*mData[id]);
            item->setId(mNextId);
            cell = std::move(item);
            mIdsInUse.insert(mNextId);
            findNextId();
            return cell.get();
        }
    }

    // id was either out of bounds of the data vector or the pointer was null
    throw std::runtime_error("cannot duplicate: item does not exist");
}

void DataListBase::remove(uint8_t id) {
    if (size() == 0) {
        throw std::runtime_error("cannot remove: list is empty");
    }

    if (id < mData.size()) {
        auto &cell = mData[id];
        if (cell) {
            mIdsInUse.erase(id);
            cell.reset();
            if (mNextId > id) {
                mNextId = id;
            }
            return;
        }
    }

    throw std::runtime_error("cannot remove: item does not exist");
}

DataItem* DataListBase::get(uint8_t id) const {
    if (id >= mData.size()) {
        return nullptr;
    } else {
        return mData[id].get();
    }
}

std::shared_ptr<DataItem> DataListBase::getShared(uint8_t id) const {
    if (id >= mData.size()) {
        return nullptr;
    } else {
        return mData[id];
    }
}

void DataListBase::findNextId() {
    if (size() < MAX_SIZE) {
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
T* DataList<T>::operator[](uint8_t id) const {
    return get(id);
}

template <class T>
T* DataList<T>::insert() {
    return static_cast<T*>(DataListBase::insert());
}

template <class T>
T* DataList<T>::insert(uint8_t id) {
    return static_cast<T*>(DataListBase::insert(id));
}

template <class T>
T* DataList<T>::duplicate(uint8_t id) {
    return static_cast<T*>(DataListBase::duplicate(id));
}
template <class T>
T* DataList<T>::get(uint8_t id) const {
    return static_cast<T*>(DataListBase::get(id));
}

template <class T>
std::shared_ptr<T> DataList<T>::getShared(uint8_t id) const {
    return std::static_pointer_cast<T>(DataListBase::getShared(id));
}

template <class T>
std::shared_ptr<DataItem> DataList<T>::createItem() {
    return std::make_shared<T>();
}

template <class T>
std::shared_ptr<DataItem> DataList<T>::copyItem(DataItem const& item) {
    return std::make_shared<T>(static_cast<T const&>(item));
}

template class DataList<Instrument>;
template class DataList<Waveform>;

}
