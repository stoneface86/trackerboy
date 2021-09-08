/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace trackerboy {


class BaseTable {

public:
    static constexpr size_t MAX_SIZE = 64;


    virtual ~BaseTable() noexcept;

    void clear() noexcept;

    //
    // total count of items in the table
    //
    size_t size() const noexcept;

    //
    // gets the next available id to insert/duplicate an item into
    //
    uint8_t nextAvailableId() const noexcept;
    
    DataItem& insert();

    DataItem& insert(uint8_t id);

    DataItem& duplicate(uint8_t id);

    DataItem const* get(uint8_t id) const;

    DataItem* get(uint8_t id);

    std::shared_ptr<DataItem> getShared(uint8_t id) const;

    void remove(uint8_t id);

protected:
    
    BaseTable() noexcept;

    virtual std::shared_ptr<DataItem> createItem() = 0;

    virtual std::shared_ptr<DataItem> copyItem(DataItem const& item) = 0;

private:

    using DataType = std::array<std::shared_ptr<DataItem>, TABLE_SIZE>;
    

    void addId(uint8_t id);

    void removeId(uint8_t id);

    void findNextId();

    DataType mData;
    size_t mSize;
    uint8_t mNextId;
};


//
// Table class. Stores instrument or waveform data. Items are stored in a fixed size
// array so that looking up via id is constant time.
//
template <class T>
class Table final : public BaseTable {

    static_assert(std::is_base_of<DataItem, T>::value, "T must inherit from DataItem");

public:

    Table();
    ~Table();

    //
    // Gets a pointer to the item with the given index if it exists. If the
    // item does not exist, nullptr is returned. The pointer may be invalidated
    // after calling insert()
    //
    T const* operator[](uint8_t id) const;

    T* operator[](uint8_t id);

    T& insert();

    T& insert(uint8_t id);

    T& duplicate(uint8_t id);

    T const* get(uint8_t id) const;

    T* get(uint8_t id);

    std::shared_ptr<T> getShared(uint8_t id) const;

protected:

    virtual std::shared_ptr<DataItem> createItem() override;

    virtual std::shared_ptr<DataItem> copyItem(DataItem const& item) override;

};

// we will only use these template instantiations

using InstrumentTable = Table<Instrument>;
using WaveformTable = Table<Waveform>;

}
