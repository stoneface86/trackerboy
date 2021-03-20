
#pragma once

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>

namespace trackerboy {


class DataListBase {

public:
    static constexpr size_t MAX_SIZE = 256;
    
    virtual ~DataListBase() noexcept;

    void clear() noexcept;

    //
    // total count of items in the list
    //
    size_t size() const noexcept;

    //
    // size of the item vector in use
    //
    size_t capacity() const noexcept;

    //
    // gets the next available id to insert/duplicate an item into
    //
    uint8_t nextAvailableId() const noexcept;

    void remove(uint8_t id);

protected:
    
    DataListBase() noexcept;

    DataItem* insertItem();

    DataItem* insertItem(uint8_t id);

    DataItem* duplicateItem(uint8_t id);

    DataItem* itemAt(uint8_t id);

    virtual DataItem* createItem() = 0;

    virtual DataItem* copyItem(DataItem const& item) = 0;

private:

    void findNextId();

    std::vector<std::unique_ptr<DataItem>> mData;
    size_t mItemCount;
    uint8_t mNextId;
};


//
// DataList class. Container for storing DataItems such that they are
// accessible via their id.
//
template <class T>
class DataList : public DataListBase {

    static_assert(std::is_base_of<DataItem, typename T>::value, "T must inherit from DataItem");

public:

    DataList();
    ~DataList();

    //
    // Gets a pointer to the item with the given index if it exists. If the
    // item does not exist, nullptr is returned. The pointer may be invalidated
    // after calling insert()
    //
    T* operator[](uint8_t id);

    T* insert();

    T* insert(uint8_t id);

    T* duplicate(uint8_t id);

protected:

    DataItem* createItem() override;

    DataItem* copyItem(DataItem const& item) override;

};

// we will only use these template instantiations
using InstrumentList = DataList<Instrument>;
using WaveformList = DataList<Waveform>;

}
