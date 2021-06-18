
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/engine/ChannelControl.hpp"

#include <stdexcept>

namespace trackerboy {


Engine::Engine(IApu &apu, Module const* mod) :
    mApu(apu),
    mModule(mod),
    mRc(),
    mMusicContext(),
    mTime(0),
    mPatternRepeat(false)
{
    if (mod != nullptr) {
        mRc.emplace(mApu, mod->instrumentTable(), mod->waveformTable());
    }
}

Module const* Engine::getModule() const {
    return mModule;
}

void Engine::setModule(Module const* mod) {
    if (mModule != mod) {
        mModule = mod;
        mMusicContext.reset();
        if (mod == nullptr) {
            mRc.reset();
        } else {
            mRc.emplace(mApu, mod->instrumentTable(), mod->waveformTable());
        }
    }
}

void Engine::reset() {
    mMusicContext.reset();
    mTime = 0;
}

void Engine::play(uint8_t orderNo, uint8_t patternRow) {
    
    if (mModule) {
        auto &song = mModule->song();

        if (orderNo >= song.order().size()) {
            throw std::invalid_argument("cannot play order, order does not exist");
        }
        if (patternRow >= song.patterns().rowSize()) {
            throw std::invalid_argument("cannot start at given row, exceeds pattern size");
        }

        mMusicContext.emplace(song, orderNo, patternRow, mPatternRepeat);
    }
}

void Engine::halt() {
    if (mMusicContext) {
        mMusicContext->halt(*mRc);
    }
}

void Engine::lock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->lock(*mRc, ch);
    } else {
        clearChannel(ch);
    }
}

void Engine::unlock(ChType ch) {
    clearChannel(ch);
    if (mMusicContext) {
        mMusicContext->unlock(*mRc, ch);
    }
}

void Engine::repeatPattern(bool repeat) {
    if (mMusicContext) {
        mMusicContext->repeatPattern(repeat);
    }
    mPatternRepeat = repeat;
}

void Engine::step(Frame &frame) {

    if (mMusicContext) {
        frame.halted = mMusicContext->step(*mRc);
        frame.startedNewRow = mMusicContext->hasNewRow();
        frame.startedNewPattern = mMusicContext->hasNewPattern();
        frame.order = mMusicContext->currentOrder();
        frame.row = mMusicContext->currentRow();
        frame.speed = mMusicContext->currentSpeed();
    } else {
        // no runtime, do nothing
        frame.halted = true;
        frame.order = 0;
        frame.row = 0;
        frame.speed = 0;
    }

    frame.time = mTime;

    // TODO: sound effects 

    // increment timestamp for next frame
    ++mTime;
}

void Engine::clearChannel(ChType ch) {
    // clear the channel (all registers + panning get zero'd)

    switch (ch) {
        case ChType::ch1:
            ChannelControl<ChType::ch1>::clear(mApu);
            break;
        case ChType::ch2:
            ChannelControl<ChType::ch2>::clear(mApu);
            break;
        case ChType::ch3:
            ChannelControl<ChType::ch3>::clear(mApu);
            break;
        case ChType::ch4:
            ChannelControl<ChType::ch4>::clear(mApu);
            break;
    }
    
}



}
