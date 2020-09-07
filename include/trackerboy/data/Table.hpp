
#pragma once

#include <array>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

// "thin-template" idiom
class BaseTable {

public:
    virtual ~BaseTable() noexcept;

    // 8-bit IDs so we can only have 256 items
    static constexpr size_t MAX_SIZE = 256;

    using Iterator = std::vector<uint8_t>::const_iterator;

    Iterator begin() const;

    void clear() noexcept;

    FormatError deserialize(std::istream &stream) noexcept;

    Iterator end() const;

    DataItem* get(uint8_t id) const;

    DataItem* getFromOrder(uint8_t order) const;

    DataItem& insertItem();

    DataItem& insertItem(uint8_t id, std::string name);

    uint8_t lookup(uint8_t order) const;

    void remove(uint8_t id);

    FormatError serialize(std::ostream &stream) noexcept;

    size_t size() const noexcept;

protected:
    BaseTable() noexcept;

    void findNextId() noexcept;

    virtual DataItem* createItem() = 0;

    // array of unique_ptr of DataItem
    // if the pointer is empty, then there is no item with id = index
    // each table will require 256 pointers, or 256 * 8 => 2048 bytes
    using ArrayType = std::array<std::unique_ptr<DataItem>, MAX_SIZE>;

    std::unique_ptr<ArrayType> mData;
    std::vector<uint8_t> mItemOrder;
    uint8_t mNextId;

};

template <class T>
class Table : public BaseTable {
    static_assert(std::is_base_of<DataItem, T>::value, "T must inherit from DataItem");

public:

    Table();
    ~Table();

    //
    // Retrieves the item located at the given index, if set. If an item is
    // not set at the index, nullptr is returned.
    //
    T* operator[](uint8_t id);

    //
    // constructs and inserts a new item at the next available id. A reference
    // to the new item is returned. If the table is full, a runtime_error will
    // be thrown
    //
    T& insert();

    //
    // insert a new item with a given name and id. If an item already exists
    // at the given id, a runtime_error will be thrown.
    //
    T& insert(uint8_t id, std::string name);

protected:

    DataItem* createItem() override;

};


typedef Table<Instrument> InstrumentTable;
typedef Table<Waveform> WaveTable;





}
