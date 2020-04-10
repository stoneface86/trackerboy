
#include "trackerboy/song/SongRuntime.hpp"


namespace trackerboy {


SongRuntime::SongRuntime(Song &song, uint8_t start) :
    mSong(song),
    mPr(song.getPattern(start), song.speed()),
    mLoadPattern(false),
    mOc(start)
{
}


bool SongRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {
    if (mLoadPattern) {
        if (++mOc >= mSong.orders().size()) {
            mOc = 0;
        }

        mPr.setPattern(mSong.getPattern(mOc));
    }

    mLoadPattern = mPr.step(synth, itable, wtable);
    return true;

}



}
