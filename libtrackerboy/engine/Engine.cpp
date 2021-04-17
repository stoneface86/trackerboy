
#include "trackerboy/engine/Engine.hpp"

#include <stdexcept>

namespace trackerboy {


Engine::Engine(IApu &apu, Module &mod) :
    mApu(apu),
    mModule(mod),
    mRc(apu, mod.instrumentTable(), mod.waveformTable()),
    mMusicContext(),
    mTime(0)
{
}

void Engine::reset() {
    mMusicContext.reset();
    mTime = 0;
}

void Engine::play(uint8_t orderNo, uint8_t patternRow) {
    
    auto &song = mModule.song();

    if (orderNo >= song.order().size()) {
        throw std::invalid_argument("cannot play order, order does not exist");
    }
    if (patternRow >= song.patterns().rowSize()) {
        throw std::invalid_argument("cannot start at given row, exceeds pattern size");
    }

    mMusicContext.emplace(song, orderNo, patternRow);
}

void Engine::halt() {
    if (mMusicContext) {
        mMusicContext->halt();
    }
}

void Engine::lock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->lock(mRc, ch);
    }
}

void Engine::unlock(ChType ch) {
    if (mMusicContext) {
        mMusicContext->unlock(mRc, ch);
    }
}

void Engine::step(Frame &frame) {

    if (mMusicContext) {
        frame.halted = mMusicContext->step(mRc);
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
