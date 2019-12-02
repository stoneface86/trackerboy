
#include "trackerboy/Table.hpp"
#include "trackerboy/fileformat.hpp"

#include <algorithm>


namespace trackerboy {


template <class T>
Table<T>::TableItem::TableItem() :
    name(""),
    value() 
{        
}


template <class T>
Table<T>::Table() :
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
void Table<T>::serialize(std::ofstream &stream) {


    // calls stream.write with a casted ptr
    #define ezwrite(ptr, size) stream.write(reinterpret_cast<const char*>(ptr), size)

    TableHeader header;
    std::copy_n(FILE_TABLE_SIGNATURE, 4, header.signature);
    header.revision = FILE_REVISION;
    header.tableSize = static_cast<uint8_t>(mData.size());
    header.tableType = T::TABLE_CODE;
    // the offset is immediately after the header
    header.tableOffset = toLittleEndian(static_cast<uint32_t>(sizeof(header) + stream.tellp()));


    // write the header
    ezwrite(&header, sizeof(header));

    // table data

    for (auto iter = mData.begin(); iter != mData.end(); ++iter) {
        // identifier
        ezwrite(&iter->first, 1);
        // name
        std::string name = iter->second.name;
        ezwrite(name.c_str(), name.size() + 1);
        // data
        iter->second.value.serialize(stream);
        
    }

    // terminator
    ezwrite(FILE_TERMINATOR, 3);

    #undef ezwrite
}

template <class T>
void Table<T>::set(uint8_t id, T &data) {
    
    bool isNew = mData.find(id) == mData.end();

    TableItem &item = mData[id];
    item.value = data;
    if (isNew) {
        item.name = "Untitled " + std::to_string(id);
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
}


template <class T>
void Table<T>::remove(uint8_t id) {
    auto iter = mData.find(id);
    if (iter != mData.end()) {
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