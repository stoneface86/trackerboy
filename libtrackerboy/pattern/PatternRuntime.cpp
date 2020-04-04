
#include "trackerboy/Table.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"


namespace trackerboy {


PatternRuntime::PatternRuntime(uint8_t speed) :
    mPattern(nullptr),
    mSpeed(speed),
    mFc(speed - Q53_make(1, 0)),
    mTr1(),
    mTr2(),
    mTr3(),
    mTr4()
{
}

void PatternRuntime::reset() {
    init();
    mTr1.reset();
    mTr2.reset();
    mTr3.reset();
    mTr4.reset();
}

void PatternRuntime::setPattern(Pattern *pattern) {
    mPattern = pattern;
    init();
}


void PatternRuntime::setSpeed(Q53 speed) {
    if (speed < Q53_make(1, 0) || speed > Q53_make(31, 0)) {
        throw std::invalid_argument("speed must be in the range of 1.0 to 31.0");
    }
    mSpeed = speed;
}


bool PatternRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    if (mIter == mEnd) {
        // end of pattern, stop
        return true;
    }

    mFc += Q53_make(1, 0); // add 1.0 (00001000)

    if (mFc >= mSpeed) {
        // new frame

        mTr1.setRow(*mIter++, itable);
        mTr2.setRow(*mIter++, itable);
        mTr3.setRow(*mIter++, itable);
        mTr4.setRow(*mIter++, itable);

        // check for pattern effect (pattern skip, speed change)
        // certain effects apply to the pattern and not the track
        // if there are conflicting effects the last one found is used


        mFc -= mSpeed;
    }

    mTr1.step(synth, wtable);
    mTr2.step(synth, wtable);
    mTr3.step(synth, wtable);
    mTr4.step(synth, wtable);
    
    return mIter == mEnd;
}

void PatternRuntime::init() {
    if (mPattern != nullptr) {
        mIter = mPattern->begin();
        mEnd = mPattern->end();
    }
    mFc = mSpeed - Q53_make(1, 0);
}


}
