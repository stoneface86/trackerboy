
#pragma once

#include <cstdint>

#include "trackerboy/data/Song.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/engine/PatternCursor.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"
#include "trackerboy/engine/Timer.hpp"

#define BIT(x) (1 << x)


namespace trackerboy {


//
// A MusicRuntime is responsible for playing "music" or pattern data on locked channels.
// The runtime will modify the given synth each frame via the step() method. The runtime
// plays indefinitely unless it is halted (pattern effect B00). A MusicRuntime can only play
// one song for its entire lifetime.
//
class MusicRuntime {

public:

    //
    // Constructs a runtime with the given context and prepares to play the given song
    // starting at a specific order and row.
    //
    MusicRuntime(RuntimeContext rc, Song &song, uint8_t orderNo, uint8_t patternRow);

    //
    // Step the runtime for a single frame. If the runtime was halted, true is returned.
    //
    bool step();

private:

    //
    // Read the current row pointed by mCursor and apply its data to the runtime,
    // starting with track 1 (CH1). This method is called recursively, ending at
    // track 4 (CH4). If the runtime should quit early (ie halt effect), true is
    // returned, false otherwise.
    //
    template <ChType ch = ChType::ch1>
    bool setRows();

    //
    // Apply the given pattern effect. Returns true if the runtime should quit early.
    //
    bool processPatternEffect(Effect effect);

    //
    // Apply the given track effect for a track.
    //
    template <ChType ch>
    void processTrackEffect(Effect effect);

    //
    // Apply the given frequency effect for the given FrequencyControl instance. True
    // is returned for effects that require apply() to be called, false otherwise.
    //
    bool processFreqEffect(Effect effect, FrequencyControl &fc);

    //
    // Update channel registers for the given channel. The NoteControl for this
    // channel is stepped and the panning mask is updated when a note is triggered/cut.
    //
    template <ChType ch>
    void update();

    template <ChType ch>
    void setEnvelope(uint8_t envelope);

    template <ChType ch>
    void setTimbre(uint8_t timbre);

    template <ChType ch>
    void setPanning(uint8_t panning);

    //
    // Write the given envelope to the channel's registers. For channels 1, 2 and 4 this
    // value is written to the channel's envelope register (NRx2). For channel 3, the
    // waveram is set to the waveform in the wave table with the envelope value being the index.
    // The channel is then restarted.
    //
    template <ChType ch>
    void writeEnvelope(uint8_t envelope);

    //
    // Write the given timbre to the channel's registers. A timbre ranges from 0-3 and its
    // effect depends on the channel.
    //
    // CH1, CH2: the duty is set (0 = 12.5%, 1 = 25%, 2 = 50%, 3 = 75%)
    // CH3:      the volume is set (0 = mute, 1 = 25%, 2 = 50%, 3 = 100%)
    // CH4:      the step-width is set (0 = 15-bit, 1,2,3 = 7-bit)
    //
    template <ChType ch>
    void writeTimbre(uint8_t timbre);

    static constexpr int FLAGS_HALTED = BIT(0);
    static constexpr int FLAGS_PANNING = BIT(1);
    static constexpr int FLAGS_AREN1 = BIT(4);
    static constexpr int FLAGS_AREN2 = BIT(5);
    static constexpr int FLAGS_AREN3 = BIT(6);
    static constexpr int FLAGS_AREN4 = BIT(7);

    static constexpr int FLAGS_DEFAULT = FLAGS_PANNING;

    enum class PatternCommand {
        none,
        next,
        jump
    };

    // context
    RuntimeContext mRc;

    Song &mSong;

    uint8_t mOrderCounter;
    // last order index for the song
    uint8_t const mLastOrder;
    uint16_t const mRowsPerTrack;

    PatternCommand mCommand;
    uint8_t mCommandParam;

    // channel settings
    uint8_t mEnvelope[4];
    uint8_t mTimbre[4];
    uint8_t mPanning;
    uint8_t mPanningMask;
    uint8_t mNoteDelay;

    int mFlags;

    // runtime components
    NoteControl mNc[4];
    FrequencyControl mFc[3];
    Timer mTimer;
    PatternCursor mCursor;

    
};


}