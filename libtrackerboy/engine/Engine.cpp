
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
}

void Engine::play(int orderNo, int patternRow) {
    
    if (mModule) {
        auto &song = mModule->song();

        if (orderNo < 0 || orderNo >= song.order().size()) {
            throw std::invalid_argument("cannot play order, order does not exist");
        }
        if (patternRow < 0 || patternRow >= song.patterns().rowSize()) {
            throw std::invalid_argument("cannot start at given row, exceeds pattern size");
        }

        mMusicContext.emplace(song, orderNo, patternRow, mPatternRepeat);
        mTime = 0;
    }
}

void Engine::halt() {
    if (mMusicContext) {
        mMusicContext->halt(*mRc);
    }
}

void Engine::jump(int pattern) {
    if (mMusicContext) {
        mMusicContext->jump(pattern);
    }
}

void Engine::lock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->lock(*mRc, ch);
    } else {
        clearChannel(ch);
    }
}

void Engine::reload() {
    if (mMusicContext) {
        mMusicContext->reloadAll(*mRc);
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
        frame.time = mTime;
        frame.halted = mMusicContext->step(*mRc, frame);

        // increment timestamp for next frame
        if (!frame.halted) {
            ++mTime;
        }
    } else {
        frame.halted = true;
    }
    // TODO: sound effects 

    
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
