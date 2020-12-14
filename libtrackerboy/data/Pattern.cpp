
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

PatternRow Pattern::operator[](uint16_t row) {
    return {
        (*mTrack1)[row], (*mTrack2)[row], (*mTrack3)[row], (*mTrack4)[row]
    };
}

unsigned Pattern::totalRows() {
    if (mRowCount == 0) {

        std::array iters = { mTrack1->begin(), mTrack2->begin(), mTrack3->begin(), mTrack4->begin() };
        auto end = mTrack1->end();

        do {
            ++mRowCount;

            for (auto &iter : iters) {

                // check for effects Bxx, C00 or D00
                for (int i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
                    if (!!(iter->flags & (TrackRow::COLUMN_EFFECT1 << i))) {
                        auto effect = iter->effects[i];
                        if (effect.type == EffectType::patternGoto || 
                            effect.type == EffectType::patternHalt || 
                            effect.type == EffectType::patternSkip) {
                            // these effects stop this pattern short, return the count here
                            return mRowCount;
                        }
                    }
                }

                ++iter;
            }

            

        } while (iters[0] != end);
        
    }

    return mRowCount;
}


}