
#include "trackerboy/engine/SongRuntime.hpp"


namespace trackerboy {


SongRuntime::SongRuntime(Song &song, uint8_t start) :
    mSong(song),
    mPr(song.getPattern(start), song.speed(), song.orders().size() - 1),
    mLoadPattern(false),
    mOc(start)
{
}


bool SongRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    switch (mPr.status()) {
        
        case PatternRuntime::State::ready:
            break;
        case PatternRuntime::State::next:
            if (++mOc >= mSong.orders().size()) {
                mOc = 0;
            }
            mPr.setPattern(mSong.getPattern(mOc));
            break;
        case PatternRuntime::State::jump:
            mPr.setPattern(mSong.getPattern(mPr.jumpPattern()));
            break;
        case PatternRuntime::State::halt:
            return false;

    }


    mPr.step(synth, itable, wtable);
    return mPr.status() != PatternRuntime::State::halt;

}



}
