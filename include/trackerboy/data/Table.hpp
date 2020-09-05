
#pragma once

#include <array>
#include <fstream>
#include <string>

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Song.hpp"
#include "trackerboy/synth/Waveform.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {


// "thin-template" idiom
class BaseTable {

public:

    struct Mapping {
        uint8_t index;
        std::string name;
    };

    using Iterator = std::vector<Mapping>::const_iterator;

    // 8-bit IDs so we can only have 256 items
    static constexpr size_t MAX_SIZE = 256;

    // iterator for the mapping vecotr
    Iterator begin();

    Iterator end();

    uint8_t lastIndex();

    //
    // Get the name of the item at the given id.
    //
    std::string name(uint8_t id);

    //
    // Set a new name for the given item if exists.
    //
    void setName(uint8_t id, std::string name);

    size_t size();

protected:

    BaseTable();

    void clear();

    void findNextId();

    std::array<uint8_t, MAX_SIZE> mMap;
    std::vector<Mapping> mMapList;
    uint8_t mNextId;
    uint8_t mLastId;


};



template <class T>
class Table : public BaseTable {

public:

    

    Table();
    ~Table();
    //Table(Table<T> &table);

    //
    // Retrieves the item located at the given index, if set. If an item is
    // not set at the index, nullptr is returned.
    //
    T* operator[](uint8_t id);

    void clear();

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

    

    void remove(uint8_t id);

private:

    std::vector<T> mItems;


};


typedef Table<Instrument> InstrumentTable;
typedef Table<Waveform> WaveTable;





}
