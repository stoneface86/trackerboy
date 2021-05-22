
#include "trackerboy/data/TrackRow.hpp"

#include <cassert>

namespace trackerboy {

std::optional<uint8_t> TrackRow::queryNote() const noexcept {
    if (note) {
        return note - 1;
    } else {
        return {};
    }
}

std::optional<uint8_t> TrackRow::queryInstrument() const noexcept {
    if (instrumentId) {
        return instrumentId - 1;
    } else {
        return {};
    }
}

std::optional<Effect> TrackRow::queryEffect(size_t effectNo) const noexcept {
    assert(effectNo < MAX_EFFECTS);
    if (effects[effectNo].type != EffectType::noEffect) {
        return effects[effectNo];
    } else {
        return {};
    }
}

bool TrackRow::isEmpty() const noexcept {
    return note == 0 && 
        instrumentId == 0 &&
        effects[0].type == EffectType::noEffect &&
        effects[1].type == EffectType::noEffect &&
        effects[2].type == EffectType::noEffect;
}

void TrackRow::setNote(std::optional<uint8_t> note_) {
    note = note_.value_or((uint8_t)-1) + 1;
}

void TrackRow::setInstrument(std::optional<uint8_t> instrument) {
    instrumentId = instrument.value_or((uint8_t)-1) + 1;
}

}
