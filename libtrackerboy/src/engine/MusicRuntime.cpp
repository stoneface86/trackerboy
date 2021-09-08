
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/engine/ChannelControl.hpp"

#include "internal/enumutils.hpp"

namespace trackerboy {

MusicRuntime::MusicRuntime(Song const& song, int orderNo, int patternRow, bool patternRepeat) :
    mSong(song),
    mOrderCounter(orderNo),
    mRowCounter(patternRow),
    mPatternRepeat(patternRepeat),
    mTimer(),
    mGlobal(),
    mFlags(DEFAULT_FLAGS),
    mStates{
        ChannelState(ChType::ch1),
        ChannelState(ChType::ch2),
        ChannelState(ChType::ch3),
        ChannelState(ChType::ch4)
    },
    mTc1(ChType::ch1),
    mTc2(ChType::ch2),
    mTc3(ChType::ch3),
    mTc4()
{
    mTimer.setPeriod(song.speed());
}

void MusicRuntime::halt(RuntimeContext const &rc) {
    mFlags.set(FLAG_HALT);
    haltChannels(rc);
}

void MusicRuntime::jump(int pattern) {
    mOrderCounter = pattern;
    mRowCounter = 0;
}

void MusicRuntime::lock(RuntimeContext const& rc, ChType ch) {
    // do nothing if channel is already locked
    if (mFlags.test(+ch)) {
        reload(rc, ch);
        // update lock bit for channel
        mFlags.reset(+ch);
    }
}

void MusicRuntime::reloadAll(RuntimeContext const& rc) {
    for (int i = +ChType::ch1; i <= +ChType::ch4; ++i) {
        if (!mFlags.test(i)) {
            reload(rc, static_cast<ChType>(i));
        }
    }
}

void MusicRuntime::reload(RuntimeContext const& rc, ChType ch) {
    // reload current channel state
    switch (ch) {
        case ChType::ch1:
            ChannelControl<ChType::ch1>::init(rc.apu, rc.waveTable, mStates[0]);
            break;
        case ChType::ch2:
            ChannelControl<ChType::ch2>::init(rc.apu, rc.waveTable, mStates[1]);
            break;
        case ChType::ch3:
            ChannelControl<ChType::ch3>::init(rc.apu, rc.waveTable, mStates[2]);
            break;
        case ChType::ch4:
            ChannelControl<ChType::ch4>::init(rc.apu, rc.waveTable, mStates[3]);
            break;
    }
    
}

void MusicRuntime::unlock(RuntimeContext const& rc, ChType ch) {
    (void)rc;
    mFlags.set(+ch);
}

void MusicRuntime::repeatPattern(bool repeat) {
    mPatternRepeat = repeat;
}

bool MusicRuntime::step(RuntimeContext const& rc, Frame &frame) {
    if (mFlags.test(FLAG_HALT)) {
        // runtime is halted, do nothing
        return true;
    }
    
    if (mFlags.test(FLAG_INIT)) {
        ChannelControl<ChType::ch1>::init(rc.apu, rc.waveTable, mStates[0]);
        ChannelControl<ChType::ch2>::init(rc.apu, rc.waveTable, mStates[1]);
        ChannelControl<ChType::ch3>::init(rc.apu, rc.waveTable, mStates[2]);
        ChannelControl<ChType::ch4>::init(rc.apu, rc.waveTable, mStates[3]);

        mFlags.reset(FLAG_INIT);
    }

    // if timer is active, we are starting a new row
    frame.startedNewRow = mTimer.active();
    // this gets set to true if:
    //  1. we have started a new row
    //  2. a pattern command was set (jump or next)
    frame.startedNewPattern = false;
    if (frame.startedNewRow) {

        // change the current pattern if needed
        if (mGlobal.patternCommand != Operation::PatternCommand::none && mPatternRepeat) {
            mGlobal.patternCommand = Operation::PatternCommand::none;
            mRowCounter = 0;
        } else {
            switch (mGlobal.patternCommand) {
                case Operation::PatternCommand::none:
                    break;
                case Operation::PatternCommand::next:
                    if (++mOrderCounter >= mSong.order().size()) {
                        // loop back to the first pattern
                        mOrderCounter = 0;
                    }
                    mRowCounter = mGlobal.patternCommandParam;
                    mGlobal.patternCommand = Operation::PatternCommand::none;
                    frame.startedNewPattern = true;
                    break;
                case Operation::PatternCommand::jump:
                    mRowCounter = 0;
                    // if the parameter goes past the last one, use the last one
                    mOrderCounter = std::min(mGlobal.patternCommandParam, (uint8_t)(mSong.order().size() - 1));
                    mGlobal.patternCommand = Operation::PatternCommand::none;
                    frame.startedNewPattern = true;
                    break;
            }
        }
        
        // set row data to our track controls
        mTc1.setRow(mSong.getRow(ChType::ch1, mOrderCounter, mRowCounter));
        mTc2.setRow(mSong.getRow(ChType::ch2, mOrderCounter, mRowCounter));
        mTc3.setRow(mSong.getRow(ChType::ch3, mOrderCounter, mRowCounter));
        mTc4.setRow(mSong.getRow(ChType::ch4, mOrderCounter, mRowCounter));
        
        if (mGlobal.halt) {
            halt(rc);
            // halting is immediate, do not continue this row
            return true;
        }

        frame.row = mRowCounter;
        frame.order = mOrderCounter;
        
    }

    // update channel state and write to registers on locked channels
    update(rc);

    // change the speed if the Fxx effect was used
    if (mGlobal.speed) {
        mTimer.setPeriod(mGlobal.speed);
        mGlobal.speed = 0;
    }
    frame.speed = mTimer.period();

    if (mTimer.step()) {
        // timer overflow, advance row counter
        if (++mRowCounter >= mSong.patterns().length()) {
            // end of pattern
            if (mGlobal.patternCommand == Operation::PatternCommand::none) {
                // load the next one if no command was set
                mGlobal.patternCommand = Operation::PatternCommand::next;
                mGlobal.patternCommandParam = 0;
            }
        }

    }

    // runtime did not halt
    return false;

}

template <ChType ch>
void MusicRuntime::update(RuntimeContext const& rc) {

    // initial current state with the previous state
    ChannelState state = mStates[+ch];

    switch (ch) {
        case ChType::ch1:
            mTc1.step(rc, state, mGlobal);
            break;
        case ChType::ch2:
            mTc2.step(rc, state, mGlobal);
            break;
        case ChType::ch3:
            mTc3.step(rc, state, mGlobal);
            break;
        case ChType::ch4:
            mTc4.step(rc, state, mGlobal);
            break;
    }
    if (!mFlags.test(+ch)) {
        // only write to registers if the channel is locked
        // unlocked channels have sfx playing on them or are being used for something else
        ChannelControl<ch>::update(rc.apu, rc.waveTable, mStates[+ch], state);
    }
    state.retrigger = false;
    // save the current state
    mStates[+ch] = state;

    // recurse and update the next channel
    if constexpr (ch != ChType::ch4) {
        update<static_cast<ChType>(+ch + 1)>(rc);
    }
}

template <ChType ch>
void MusicRuntime::haltChannels(RuntimeContext const& rc) {
    // zero out the channel
    ChannelState state;

    // only write if the channel is locked
    if (!mFlags.test(+ch)) {
        ChannelControl<ch>::update(rc.apu, rc.waveTable, mStates[+ch], state);
    }
    // save the state
    mStates[+ch] = state;
    // recurse and halt the next channel
    if constexpr (ch != ChType::ch4) {
        haltChannels<static_cast<ChType>(+ch + 1)>(rc);
    }
}



}
