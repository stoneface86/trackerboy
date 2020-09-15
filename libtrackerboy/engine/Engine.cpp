
#include "trackerboy/engine/Engine.hpp"


namespace trackerboy {


Engine::Engine(RuntimeContext rc) :
    mRc(rc),
    mMusicContext(std::nullopt),
    mChflags(0)
{
}

void Engine::reset() {
    mMusicContext.reset();
    mChflags = 0;
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

bool Engine::step() {

    bool halted = false;

    if (mMusicContext) {
        auto &ctx = mMusicContext.value();
        halted = ctx.step();
    }

    // TODO: sound effects 

    return halted;
}



}
