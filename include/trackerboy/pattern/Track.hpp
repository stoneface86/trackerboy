
#pragma once

#include "trackerboy/pattern/TrackData.hpp"
#include "trackerboy/pattern/TrackRow.hpp"


namespace trackerboy {


// Track class provides access to the pattern data for a specific location
// in the master pattern.

class Track {

public:

    Track(TrackData::iterator begin, TrackData::iterator end);

    TrackData::iterator begin();

    void clear(uint8_t rowStart, uint8_t rowEnd);

    void clearEffect(uint8_t row, uint8_t effectNo);

    void clearInstrument(uint8_t row);

    void clearNote(uint8_t row);

    TrackData::iterator end();

    void setEffect(uint8_t row, uint8_t effectNo, EffectType effect, uint8_t param = 0);

    void setInstrument(uint8_t row, uint8_t instrumentId);

    void setNote(uint8_t row, uint8_t note);

private:

    TrackData::iterator mBegin;
    TrackData::iterator mEnd;



};



}
