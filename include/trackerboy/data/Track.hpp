
#pragma once

#include <vector>

#include "trackerboy/data/TrackRow.hpp"


namespace trackerboy {


// container class for track data

class Track {

public:

    using Data = std::vector<TrackRow>;

    Track(uint16_t rows);

    Data::iterator begin();

    void clear(uint16_t rowStart, uint16_t rowEnd);

    void clearEffect(uint8_t row, uint8_t effectNo);

    void clearInstrument(uint8_t row);

    void clearNote(uint8_t row);

    Data::iterator end();

    void setEffect(uint8_t row, uint8_t effectNo, EffectType effect, uint8_t param = 0);

    void setInstrument(uint8_t row, uint8_t instrumentId);

    void setNote(uint8_t row, uint8_t note);

    void replace(uint8_t rowno, TrackRow &row);

    void resize(uint16_t newSize);

    uint16_t rowCount();

private:

    uint16_t mRowCounter; // how many rows are set
    Data mData;

    template <bool clear>
    TrackRow& updateColumns(uint8_t rowNo, uint8_t columns);

};



}
