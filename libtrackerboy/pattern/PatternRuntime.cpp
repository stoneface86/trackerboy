
#include "trackerboy/Table.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"

#include <utility>

namespace trackerboy {


PatternRuntime::PatternRuntime(Pattern &&pattern, uint8_t speed) :
    mSpeed(speed),
    mFc(speed - Q53_make(1, 0)),
    mTr1(),
    mTr2(),
    mTr3(),
    mTr4()
{
    setPattern(std::move(pattern));
}

void PatternRuntime::setPattern(Pattern &&pattern) {
    mIter1 = std::get<0>(pattern).begin();
    mIter2 = std::get<1>(pattern).begin();
    mIter3 = std::get<2>(pattern).begin();
    mIter4 = std::get<3>(pattern).begin();
    mEnd1 = std::get<0>(pattern).end();
    mFc = mSpeed - Q53_make(1, 0);
}


void PatternRuntime::setSpeed(Q53 speed) {
    if (speed < Q53_make(1, 0) || speed > Q53_make(31, 0)) {
        throw std::invalid_argument("speed must be in the range of 1.0 to 31.0");
    }
    mSpeed = speed;
}


bool PatternRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    if (mIter1 == mEnd1) {
        // end of pattern, stop
        return true;
    }

    mFc += Q53_make(1, 0); // add 1.0 (00001000)

    if (mFc >= mSpeed) {
        // new frame

        mTr1.setRow(*(mIter1++), itable);
        mTr2.setRow(*(mIter2++), itable);
        mTr3.setRow(*(mIter3++), itable);
        mTr4.setRow(*(mIter4++), itable);

        // check for pattern effect (pattern skip, speed change)
        // certain effects apply to the pattern and not the track
        // if there are conflicting effects the last one found is used


        mFc -= mSpeed;
    }

    mTr1.step(synth, wtable);
    mTr2.step(synth, wtable);
    mTr3.step(synth, wtable);
    mTr4.step(synth, wtable);
    
    return mIter1 == mEnd1;
}


}
