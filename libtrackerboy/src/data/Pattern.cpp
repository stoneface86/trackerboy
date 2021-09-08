
#include "trackerboy/data/Pattern.hpp"


namespace trackerboy {


Pattern::Pattern(Track &track1, Track &track2, Track &track3, Track &track4) :
    mTrack1(&track1),
    mTrack2(&track2),
    mTrack3(&track3),
    mTrack4(&track4),
    mRowCount(0)
{
}

PatternRow Pattern::operator[](int row) {
    return {
        (*mTrack1)[row], (*mTrack2)[row], (*mTrack3)[row], (*mTrack4)[row]
    };
}

TrackRow& Pattern::getTrackRow(ChType ch, int row) {
    Track *track;
    switch (ch) {
        case ChType::ch1:
            track = mTrack1;
            break;
        case ChType::ch2:
            track = mTrack2;
            break;
        case ChType::ch3:
            track = mTrack3;
            break;
        default:
            // default is used because gcc complains that track may be uninitialized
            track = mTrack4;
            break;
    }
    return track->operator[](row);
}

TrackRow const& Pattern::getTrackRow(ChType ch, int row) const {
    return const_cast<TrackRow const&>(
        const_cast<Pattern*>(this)->getTrackRow(ch, row)
        );
}

int Pattern::size() const {
    return mTrack1->size();
}

int Pattern::totalRows() {
    if (mRowCount == 0) {

        std::array iters = { mTrack1->begin(), mTrack2->begin(), mTrack3->begin(), mTrack4->begin() };
        auto end = mTrack1->end();

        do {
            ++mRowCount;

            for (auto &iter : iters) {

                // check for effects Bxx, C00 or D00
                for (int i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
                    auto effect = iter->effects[i];
                    if (effectTypeShortensPattern(effect.type)) {
                        // these effects stop this pattern short, return the count here
                        return mRowCount;
                    }
                }

                ++iter;
            }

            

        } while (iters[0] != end);
        
    }

    return mRowCount;
}


}
