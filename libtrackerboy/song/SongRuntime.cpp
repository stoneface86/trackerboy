
#include "trackerboy/song/SongRuntime.hpp"


namespace trackerboy {


SongRuntime::SongRuntime() :
    mSong(nullptr),
    mPlaying(false),
    mLoadPattern(true)
{
}

void SongRuntime::reset() {
    mPr.reset();
    mPlaying = false;
    mLoadPattern = true;
}

void SongRuntime::play(size_t loopCount) {
    Order &order = mSong->order();
    if (loopCount == LOOP_INFINITE) {
        mSequencer = Order::Sequencer(order);
    } else {
        mSequencer = Order::Sequencer(order, loopCount);
    }
    mPlaying = true;
    mLoadPattern = true;
    mPr.setSpeed(mSong->speed());
}

void SongRuntime::playOnce() {
    mSequencer = Order::Sequencer(mSong->order(), 0);
    mPlaying = true;
    mLoadPattern = true;
    mPr.setSpeed(mSong->speed());
}

void SongRuntime::setSong(Song *song) {
    mSong = song;
    reset();
}

bool SongRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {
    if (mPlaying) {
        if (mLoadPattern) {
            if (mSequencer.hasNext()) {
                uint8_t index = mSequencer.next();
                auto &patterns = mSong->patterns();
                if (index >= patterns.size()) {
                    throw std::runtime_error("invalid pattern index in order");
                }
                mPr.setPattern(&patterns[index]);
            } else {
                mPlaying = false;
                return false;
            }
        }

        mLoadPattern = mPr.step(synth, itable, wtable);
        return true;

    } else {
        return false;
    }
}



}