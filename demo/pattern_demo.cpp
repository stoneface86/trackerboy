

#include "trackerboy/export/Wav.hpp"
#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/PatternMaster.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"
#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/note.hpp"

#include "audio.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>

using audio::PlaybackQueue;

using namespace trackerboy;

static constexpr float SAMPLING_RATE = 44100;


int main() {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    Synth synth(SAMPLING_RATE);
    PlaybackQueue pb(SAMPLING_RATE);

    InstrumentTable itable;
    WaveTable wtable;

    {
        Instrument &inst = itable.insert(0, "CH1 Initial");
        auto &prgm = inst.getProgram();
        // Frame 0: retrigger, env = 0xF1, duty = 0x2
        prgm.push_back(0x6D); // ctrl: xctrl, aux2, duty = 1
        prgm.push_back(0x45); // xctrl byte: panning = L
        prgm.push_back(0x01);
        prgm.push_back(0x77); // aux2: env = 0x77
    }

    {
        Instrument &inst = itable.insert(1, "CH2 Initial");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x6D); // ctrl: xctrl, aux2, duty = 1
        prgm.push_back(0x06); // xctrl byte: panning = R
        prgm.push_back(0xA7); // aux2: env = 0xA7
    }

    {
        Instrument &inst = itable.insert(2, "CH3 Lead");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x0D); // ctrl: aux2, wave volume = 0x1
        prgm.push_back(0x00); // aux2: wave id = 0

        Waveform &wave2 = wtable.insert(0, "curved triangle");
        wave2.fromString("02468ACEEFFFFEEEDDCBA98765432211");
    }

    {
        Instrument &inst = itable.insert(3, "envelope=47");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x48);
        prgm.push_back(0x47);
    }

    {
        Instrument &inst = itable.insert(4, "envelope=57");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x48);
        prgm.push_back(0x57);
    }

    {
        Instrument &inst = itable.insert(5, "envelope=77");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x48);
        prgm.push_back(0x77);
    }

    {
        Instrument &inst = itable.insert(6, "envelope=97");
        auto &prgm = inst.getProgram();
        prgm.push_back(0x48);
        prgm.push_back(0x97);
    }

    // The following is a re-creation of national park from pokemon G/S/C
    // Note data, volume and tempo should be 100% accurate with the game
    // Only differences should be timing and vibrato (none so far currently)

    // Pattern container with 64-row patterns
    PatternMaster pm(64);


    #define note(n, o) trackerboy::NOTE_##n + trackerboy::OCTAVE_##o

    #if 1
    Track &tr1 = pm.getTrack(ChType::ch1, 0);
    tr1.setInstrument(2, 0);
    tr1.setNote(0x02, note(Ab, 3));
    tr1.setNote(0x08, note(F, 4));
    tr1.setInstrument(0x0A, 3);
    tr1.setNote(0x0A, note(F, 4));
    tr1.setNote(0x0B, note(Ab, 4));
    tr1.setInstrument(0x0C, 4);
    tr1.setNote(0x0C, note(F, 4));
    tr1.setNote(0x0D, note(Ab, 4));
    tr1.setInstrument(0x0E, 5);
    tr1.setNote(0x0E, note(F, 4));
    tr1.setNote(0x0F, note(Ab, 4));
    tr1.setInstrument(0x10, 6);
    tr1.setNote(0x10, note(F, 4));
    tr1.setNote(0x11, note(Ab, 4));
    
    tr1.setInstrument(0x12, 5);
    tr1.setNote(0x12, note(A, 3));
    tr1.setNote(0x18, note(Gb, 4));
    tr1.setInstrument(0x1A, 3);
    tr1.setNote(0x1A, note(Gb, 4));
    tr1.setNote(0x1B, note(A, 4));
    tr1.setInstrument(0x1C, 4);
    tr1.setNote(0x1C, note(Gb, 4));
    tr1.setNote(0x1D, note(A, 4));
    tr1.setInstrument(0x1E, 5);
    tr1.setNote(0x1E, note(Gb, 4));
    tr1.setNote(0x1F, note(A, 4));
    tr1.setInstrument(0x20, 6);
    tr1.setNote(0x20, note(Gb, 4));
    tr1.setNote(0x21, note(A, 4));
    tr1.setInstrument(0x22, 5);
    tr1.setNote(0x22, note(Ab, 3));
    tr1.setNote(0x28, note(F, 4));
    tr1.setInstrument(0x2A, 3);
    tr1.setNote(0x2A, note(C, 5));
    tr1.setNote(0x2B, note(Db, 5));
    tr1.setInstrument(0x2C, 4);
    tr1.setNote(0x2C, note(C, 5));
    tr1.setNote(0x2D, note(Db, 5));
    tr1.setInstrument(0x2E, 5);
    tr1.setNote(0x2E, note(C, 5));
    tr1.setNote(0x2F, note(Db, 5));
    tr1.setInstrument(0x30, 6);
    tr1.setNote(0x30, note(C, 5));
    tr1.setNote(0x31, note(Db, 5));
    tr1.setInstrument(0x32, 5);
    tr1.setNote(0x32, note(A, 3));
    tr1.setNote(0x38, note(Db, 4));
    tr1.setInstrument(0x3A, 3);
    tr1.setNote(0x3A, note(Db, 5));
    tr1.setNote(0x3B, note(Eb, 5));
    tr1.setInstrument(0x3C, 4);
    tr1.setNote(0x3C, note(Db, 5));
    tr1.setNote(0x3D, note(Eb, 5));
    tr1.setInstrument(0x3E, 5);
    tr1.setNote(0x3E, note(Db, 5));
    tr1.setNote(0x3F, note(Eb, 5));

    #endif

    #if 1
    
    Track &tr3 = pm.getTrack(ChType::ch3, 0);
    tr3.setInstrument(0, 2);
    tr3.setNote(0x00, note(Ab, 5));

    tr3.setNote(0x01, note(Bb, 5));

    tr3.setNote(0x02, note(C, 6));
    
    tr3.setNote(0x12, note(Db, 6));
    tr3.setNote(0x20, note(Ab, 6));

    tr3.setNote(0x21, note(Bb, 6));

    tr3.setNote(0x22, note(C, 7));

    tr3.setNote(0x32, note(Db, 7));
    tr3.setNote(0x3C, note(C, 7));
    tr3.setNote(0x3E, note(Db, 7));
    #endif

    #if 1
    Track &tr2 = pm.getTrack(ChType::ch2, 0);
    tr2.setInstrument(2, 1);
    tr2.setNote(0x02, note(Db, 3));

    
    tr2.setNote(0x08, note(Ab, 3));
    
    tr2.setNote(0x0A, note(Db, 4));
    


    tr2.setNote(0x12, note(Db, 3));
    

    tr2.setNote(0x18, note(A, 3));
    
    tr2.setNote(0x1A, note(Db, 4));

    

    tr2.setNote(0x22, note(Db, 3));
    
    tr2.setNote(0x28, note(Ab, 3));
    
    tr2.setNote(0x2A, note(Db, 4));

    tr2.setNote(0x32, note(Db, 3));
    tr2.setNote(0x38, note(A, 3));
    tr2.setNote(0x3A, note(Gb, 4));

    #endif
    
    #if 0
    // Track #0 for Channel 1 (pulse)
    Track track = pm.getTrack(ChType::ch1, 0);

    track.setNote(16, trackerboy::NOTE_C + trackerboy::OCTAVE_6);
    track.setInstrument(16, 0);

    track.setNote(18, trackerboy::NOTE_CUT);
    
    track.setNote(22, trackerboy::NOTE_B + trackerboy::OCTAVE_5);
    track.setInstrument(22, 0);

    track.setNote(24, trackerboy::NOTE_Bb + trackerboy::OCTAVE_5);
    track.setInstrument(24, 0);

    track.setNote(26, trackerboy::NOTE_CUT);

    track.setNote(30, trackerboy::NOTE_A + trackerboy::OCTAVE_5);
    track.setInstrument(30, 0);

    track.setNote(32, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    track.setInstrument(32, 0);

    track.setNote(34, trackerboy::NOTE_CUT);

    track.setNote(37, trackerboy::NOTE_Eb + trackerboy::OCTAVE_5);
    track.setInstrument(37, 0);

    track.setNote(44, trackerboy::NOTE_CUT);

    track.setNote(0x2F, trackerboy::NOTE_Ab + trackerboy::OCTAVE_5);
    track.setInstrument(0x2F, 0);

    track.setNote(0x31, trackerboy::NOTE_G + trackerboy::OCTAVE_5);
    track.setInstrument(0x31, 0);

    track.setNote(0x35, trackerboy::NOTE_CUT);

    track.setNote(0x41, trackerboy::NOTE_Gb + trackerboy::OCTAVE_6);
    track.setInstrument(0x41, 0);

    track.setNote(0x42, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
    track.setInstrument(0x42, 0);

    #endif

    // setup the runtime for playback of a pattern with track #0 for all channels
    // 0x14 is the speed, or 2.5 frames per row
    constexpr uint8_t SPEED = 0x48;
    PatternRuntime pr(pm.getPattern(0, 0, 0, 0), SPEED);
    
    std::ofstream file("pattern_demo.wav", std::ios::binary | std::ios::out);
    Wav wav(file, 2, SAMPLING_RATE);
    wav.begin();
    pb.start();

    
    bool patternEnded;
    do {
        patternEnded = pr.step(synth, itable, wtable);
        size_t framesize = synth.run();
        //outputFrame(synth.buffer(), framesize, pb);
        pb.writeAll(synth.buffer(), framesize);
        wav.write(synth.buffer(), framesize);
    } while (!patternEnded);

    pb.stop(true);

    wav.finish();
    file.close();

    Pa_Terminate();
    return 0;

}
