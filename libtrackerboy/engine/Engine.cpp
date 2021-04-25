
#include "trackerboy/engine/Engine.hpp"

#include <stdexcept>

namespace trackerboy {


Engine::Engine(IApu &apu, Module const* mod) :
    mApu(apu),
    mModule(mod),
    mRc(),
    mMusicContext(),
    mTime(0)
{
    if (mod != nullptr) {
        mRc.emplace(mApu, mod->instrumentTable(), mod->waveformTable());
    }
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

        mMusicContext.emplace(song, orderNo, patternRow);
    }
}

void Engine::halt() {
    if (mMusicContext) {
        mMusicContext->halt();
    }
}

void Engine::lock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->lock(*mRc, ch);
    }
}

void Engine::unlock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->unlock(*mRc, ch);
    }
}

void Engine::step(Frame &frame) {

    if (mMusicContext) {
        frame.halted = mMusicContext->step(*mRc);
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



}
