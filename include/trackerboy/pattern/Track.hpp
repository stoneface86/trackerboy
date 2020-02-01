
#pragma once

#include <iterator>

#include "trackerboy/pattern/TrackRow.hpp"


namespace trackerboy {


class Track {

public:

    class Iterator {

    public:
        using self_type = Iterator;
        using value_type = TrackRow;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = int;

        Iterator() = default;
        Iterator(pointer ptr);
        Iterator(const self_type &iter);
        ~Iterator();

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

    Track(TrackRow *begin, TrackRow *end);

    Iterator begin();

    void clear(uint8_t rowStart, uint8_t rowEnd);

    void clearEffect(uint8_t row, uint8_t effectNo);

    void clearInstrument(uint8_t row);

    void clearNote(uint8_t row);

    Iterator end();

    void setEffect(uint8_t row, uint8_t effectNo, EffectType effect, uint8_t param = 0);

    void setInstrument(uint8_t row, uint8_t instrumentId);

    void setNote(uint8_t row, uint8_t note);

private:

    TrackRow *mBegin;
    TrackRow *mEnd;

};



}
