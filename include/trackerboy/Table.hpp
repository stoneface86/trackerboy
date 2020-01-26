
#pragma once

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

    // 8-bit IDs so we can only have 256 items
    static constexpr size_t MAX_SIZE = 256;

    Table();

    T* operator[](uint8_t id);

    uint8_t add(T &data);

    std::string name(uint8_t id);

    void set(uint8_t id, T &data);

    void setName(uint8_t id, std::string name);

    size_t size();

    void remove(uint8_t id);

private:

    struct TableItem {
        std::string name;
        T value;

        TableItem();
    };

    uint8_t mNextId;
    std::unordered_map<uint8_t, TableItem> mData;

    void findNextId();

};


typedef Table<Instrument> InstrumentTable;
typedef Table<Song> SongTable;
typedef Table<Waveform> WaveTable;





}