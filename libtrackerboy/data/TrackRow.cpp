
#include "trackerboy/data/TrackRow.hpp"

#include <cassert>

namespace trackerboy {

std::optional<uint8_t> TrackRow::queryNote() const noexcept {
    if (!!(flags & COLUMN_NOTE)) {
        return note;
    } else {
        return {};
    }
}

std::optional<uint8_t> TrackRow::queryInstrument() const noexcept {
    if (!!(flags & COLUMN_INST)) {
        return instrumentId;
    } else {
        return {};
    }
}

std::optional<Effect> TrackRow::queryEffect(size_t effectNo) const noexcept {
    assert(effectNo < MAX_EFFECTS);
    if (!!(flags & (COLUMN_EFFECT1 << effectNo))) {
        return effects[effectNo];
    } else {
        return {};
    }
}

bool TrackRow::isEmpty() const noexcept {
    return flags == 0;
}

}
