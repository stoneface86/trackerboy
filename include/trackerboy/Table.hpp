
#pragma once

#include <unordered_map>

#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/pattern/Pattern.hpp"

namespace trackerboy {


template <class T>
class Table {

public:

    // 8-bit IDs so we can only have 256 items
    static constexpr size_t MAX_SIZE = 256;

    Table();

    T* operator[](uint8_t id);

    uint8_t add(T data);

    void remove(uint8_t id);

private:
    uint8_t nextId;
    std::unordered_map<uint8_t, T> mData;

    void findNextId();

};


typedef Table<Instrument> InstrumentTable;
typedef Table<Pattern> PatternTable;
//typedef Table<Song> SongTable;
//typedef Table<Wave> WaveTable;





}