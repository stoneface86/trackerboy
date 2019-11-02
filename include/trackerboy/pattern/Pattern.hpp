#pragma once

#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/EffectType.hpp"
#include "trackerboy/pattern/TrackRow.hpp"


namespace trackerboy {


class Pattern {

public:

    using PatternData = std::vector<TrackRow>;

    class TrackIterator {
    
    public:
        using self_type = TrackIterator;
        using value_type = TrackRow;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = int;

        TrackIterator(pointer ptr);
        TrackIterator(const self_type &iter);
        ~TrackIterator();

        self_type operator++();
        self_type operator++(int junk);
        const reference operator*();
        const pointer operator->();
        bool operator==(const self_type &rhs);
        bool operator!=(const self_type &rhs);
        self_type operator=(const self_type &rhs);

    private:
        pointer mPtr;

    };

    static constexpr size_t MAX_ROWS = 256;
    static constexpr size_t DEFAULT_ROWS = 64;

    Pattern(size_t nrows = DEFAULT_ROWS);

    PatternData::const_iterator begin();

    void clear(uint8_t rowStart, uint8_t rowEnd);

    void clearEffect(ChType trackId, uint8_t row);
    
    void clearInstrument(ChType trackId, uint8_t row);

    void clearNote(ChType trackId, uint8_t row);

    PatternData copy();

    PatternData::const_iterator end();

    void fromStream(std::vector<uint8_t> &streamBuf);

    void setEffect(ChType trackId, uint8_t row, EffectType effect, uint8_t param = 0);

    void setInstrument(ChType trackId, uint8_t row, uint8_t instrumentId);

    void setNote(ChType trackId, uint8_t row, Note note);

    void setSize(size_t nrows);

    size_t size();

    void toStream(std::vector<uint8_t> &streamBuf);

    TrackIterator trackBegin(ChType trackId);

    TrackIterator trackEnd(ChType trackId);

private:
    size_t mSize;
    PatternData mData;


};



}