
#include "trackerboy/Table.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"

#include <cassert>
#include <utility>

namespace trackerboy {

namespace {

// 1.0 = 00001.000 = 0x8
static constexpr Speed SPEED_ONE = 0x8;

}

PatternRuntime::PatternRuntime(Pattern &&pattern, Speed speed, uint8_t lastOrder) :
    mSpeed(speed),
    mFc(0),
    mLastOrder(lastOrder),
    mTr1(),
    mTr2(),
    mTr3(),
    mTr4(),
    mStatus(State::ready),
    mNextStatus(State::ready),
    mJumpTo(0)
{
    setPattern(std::move(pattern));
}

uint8_t PatternRuntime::jumpPattern() {
    return mJumpTo;
}

void PatternRuntime::setPattern(Pattern &&pattern) {
    mIter1 = std::get<0>(pattern).begin();
    mIter2 = std::get<1>(pattern).begin();
    mIter3 = std::get<2>(pattern).begin();
    mIter4 = std::get<3>(pattern).begin();
    mEnd1 = std::get<0>(pattern).end();
    //mFc = Q53_make(0, 0);
    mStatus = State::ready;
    
}


void PatternRuntime::setSpeed(Speed speed) {
    if (speed < SPEED_MIN || speed > SPEED_MAX) {
        throw std::invalid_argument("speed must be in the range of 1.0 to 31.0");
    }
    mSpeed = speed;
}

PatternRuntime::State PatternRuntime::status() {
    return mStatus;
}

void PatternRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    // how timing works
    //
    // The frame counter gets incremented by 1.0 every step
    // when the frame counter is < 1.0, this frame starts a new row
    // when the frame counter is >= mSpeed after incrementing,
    // then we need to advance the iterators and adjust the counter
    //
    // example, mSpeed = 2.5 (00010.100)
    // mFc = 0.0
    //
    // === step, Frame#0 Row#0 ===
    // mFc < 1.0, set row
    // mFc += 1.0 -> 1.0
    // === step, Frame#1 Row#0 ===
    // mFc += 1.0 -> 2.0
    // === step, Frame#2 Row#0 ===
    // mFc += 1.0 -> 3.0
    // mFc >= 2.5 so advance the iterators
    // mFc -= mSpeed -> 0.5
    // === step, Frame#3 Row#1 ===
    // mFc < 1.0, set row
    // mFc += 1.0 -> 1.5
    // === step, Frame#4 Row#1 ===
    // mFc += 1.0 -> 2.5
    // mFc >= 2.5 so advance the iterators
    // mFc -= mSpeed -> 0.0
    // === step, Frame#5 Row#2 ===
    // ...
    //
    // Row#0 gets 3 frames, Row#1 gets 2, and then repeats like that

    // we can only step if we are ready
    assert(mStatus == State::ready);

    if (mFc < SPEED_ONE) {
        // parse effects
        // here we will apply pattern effects and set aside the track effects
        parseEffects(*mIter1);
        parseEffects(*mIter2);
        parseEffects(*mIter3);
        parseEffects(*mIter4);


        // new row
        mTr1.setRow(*mIter1, itable);
        mTr2.setRow(*mIter2, itable);
        mTr3.setRow(*mIter3, itable);
        mTr4.setRow(*mIter4, itable);
    }

    // advance the frame counter
    mFc += SPEED_ONE;

    // are we the last frame before the next row?
    if (mFc >= mSpeed) {
        // yes, update frame counter
        mFc -= mSpeed;
        // advance the iterators
        if (++mIter1 == mEnd1) {
            // end of pattern, update status
            mStatus = mNextStatus == State::ready ? State::next : mNextStatus; // runtime does nothing until the next pattern is set
        } else {
            // not at the end, advance the remaining iterators
            ++mIter2;
            ++mIter3;
            ++mIter4;
            mStatus = mNextStatus;
        }
        mNextStatus = State::ready;
    }

    
    // step the track runtimes
    mTr1.step(synth, wtable);
    mTr2.step(synth, wtable);
    mTr3.step(synth, wtable);
    mTr4.step(synth, wtable);
    
    
    
}

void PatternRuntime::parseEffects(TrackRow &row) {
    for (size_t i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
        if (!!(row.flags & (TrackRow::COLUMN_EFFECT1 << i))) {
            Effect &effect = row.effects[i];
            switch (effect.type) {
                case EffectType::patternGoto:
                    if (effect.param <= mLastOrder) {
                        mNextStatus = State::jump;
                        mJumpTo = effect.param;
                    }
                    break;
                case EffectType::patternHalt:
                    // update status so we halt immediately
                    mStatus = State::halt;
                    mNextStatus = State::halt;
                    break;
                case EffectType::patternSkip:
                    mNextStatus = State::next;
                    break;
                case EffectType::setTempo:
                    if (effect.param >= SPEED_MIN && effect.param <= SPEED_MAX) {
                        mSpeed = effect.param;
                    }
                    break;
            }
        }
    }
}


}
