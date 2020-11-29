
#include "trackerboy/engine/Engine.hpp"


namespace trackerboy {


Engine::Engine(RuntimeContext rc) :
    mRc(rc),
    mMusicContext(std::nullopt),
    mTime(0)
{
}

void Engine::reset() {
    mMusicContext.reset();
    mTime = 0;
}

void Engine::play(Song &song, uint8_t orderNo, uint8_t patternRow) {
    
    auto &orders = song.orders();
    if (orderNo >= orders.size()) {
        throw std::invalid_argument("cannot play order, order does not exist");
    }
    if (patternRow >= song.patterns().rowSize()) {
        throw std::invalid_argument("cannot start at given row, exceeds pattern size");
    }

    mMusicContext.emplace(mRc, mChCtrl, song, orderNo, patternRow);
}

void Engine::lock(ChType ch) {
    mChCtrl.lock(ch);
    if (mMusicContext) {
        mMusicContext.value().reload(ch);
    }
}

void Engine::unlock(ChType ch) {
    mChCtrl.unlock(ch);
}

void Engine::step(Frame &frame) {

    if (mMusicContext) {
        auto &ctx = mMusicContext.value();
        frame.halted = ctx.step();
        frame.order = ctx.currentOrder();
        frame.row = ctx.currentRow();
        frame.speed = ctx.speed();
    } else {
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
