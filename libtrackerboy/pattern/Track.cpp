

#include "trackerboy/pattern.hpp"


namespace trackerboy {


Track::Track() 
{
}

void Track::clear(uint8_t row, TrackRow::RowFlags columns) {
    // clear the set bits in row's flags field
    mTable[row].flags &= ~columns;
}

void Track::setNote(uint8_t row, Note note) {
    TrackRow &rowdata = mTable[row];
    rowdata.note = note;
    // set the note set bit (indicates a note was set in the row)
    rowdata.flags |= TrackRow::FLAGS_NOTESET;
}

void Track::setInstrument(uint8_t row, uint8_t instrumentId) {
    TrackRow& rowdata = mTable[row];
    rowdata.instrumentId = instrumentId;
    // set the instrument set bit (indicates an instrument was set in the row)
    rowdata.flags |= TrackRow::FLAGS_INSTSET;
}

void Track::setEffect(uint8_t row, TrackRow::EffectType effect, uint8_t param) {
    TrackRow& rowdata = mTable[row];
    // set the note set bit (indicates a note was set in the row)
    uint8_t flags = rowdata.flags & 0xF; // discard old effect
    // update flags with the effect type and effect set bit
    rowdata.flags = flags | effect | TrackRow::FLAGS_EFFECTSET;
    rowdata.effect = param;
}


}