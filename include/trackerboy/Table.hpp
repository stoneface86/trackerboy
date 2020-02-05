
#pragma once

#include <array>
#include <fstream>
#include <unordered_map>
#include <string>

#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/song/Song.hpp"
#include "trackerboy/synth/Waveform.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {


template <class T>
class Table {

public:
    struct TableItem {
        uint8_t id;
        std::string name;
        T value;

        TableItem(uint8_t id, std::string name, T value);
    };

    using TableData = typename std::vector<TableItem>;
    using Iterator = typename TableData::const_iterator;

    // 8-bit IDs so we can only have 256 items
    static constexpr size_t MAX_SIZE = 256;

    Table();
    ~Table();
    //Table(Table<T> &table);

    //
    // Retrieves the item located at the given index, if set. If an item is
    // not set at the index, nullptr is returned.
    //
    T* operator[](uint8_t id);

    Iterator begin();

    void clear();

    Iterator end();

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

    //
    // Get the name of the item at the given id.
    //
    std::string name(uint8_t id);

    //
    // Set a new name for the given item if exists.
    //
    void setName(uint8_t id, std::string name);

    size_t size();

    void remove(uint8_t id);

private:

    // maps table index -> mItemData index
    std::array<uint8_t, MAX_SIZE> mMap;
    typename std::vector<TableItem> mItemData;

    uint8_t mNextId;

    void findNextId();

};


typedef Table<Instrument> InstrumentTable;
typedef Table<Song> SongTable;
typedef Table<Waveform> WaveTable;





}
