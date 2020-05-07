
#include "trackerboy/engine/Engine.hpp"


namespace trackerboy {


Engine::Engine() :
    mSong(nullptr),
    mOc(0)
{
}


void Engine::play(Song &song, uint8_t orderNo, uint8_t patternRow) {
    // reset everything
    mPr.reset();

    mSong = &song;
    auto &orders = song.orders();
    if (orderNo >= orders.size()) {
        throw std::invalid_argument("cannot play order, order does not exist");
    }

    if (!mPr) {
        mPr.emplace(song.getPattern(orderNo), song.speed(), orders.size());
    } else {
        //mPr.reset(song.getPattern(orderNo), song.speed(), orders.size());
    }

    // TODO: jump to row in pattern
    mOc = orderNo;
    
}


bool Engine::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    if (mSong == nullptr) {
        return false;
    } else {
        auto &pr = mPr.value();

        switch (pr.status()) {

            case PatternRuntime::State::ready:
                break;
            case PatternRuntime::State::next:
                if (++mOc >= mSong->orders().size()) {
                    mOc = 0;
                }
                pr.setPattern(mSong->getPattern(mOc));
                break;
            case PatternRuntime::State::jump:
                pr.setPattern(mSong->getPattern(pr.jumpPattern()));
                break;
            case PatternRuntime::State::halt:
                return false;

        }


        pr.step(synth, itable, wtable);
        return pr.status() != PatternRuntime::State::halt;
    }
}



}
