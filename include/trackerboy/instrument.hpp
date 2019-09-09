
#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>
using std::uint8_t;

#include "track.hpp"
#include "synth.hpp"


namespace trackerboy {

class ParseError : public std::runtime_error {

    size_t lineno;
    size_t column;

public:
    ParseError(size_t column, size_t lineno = 0);

    size_t getLineno();

    size_t getColumn();
};


//
// POD struct for an instrument instruction. An instruction determines
// which settings of the channel to set. Instructions are 3 bytes long.
//
struct Instruction {
    uint8_t duration;    // number of frames this instruction runs for
    uint8_t ctrl;        // control flags
    uint8_t settings;    // settings (channel specific)
    uint8_t envSettings; // envelope settings (except for channel 3)
    uint8_t note;        // note/frequency to set (channels 1, 2, 3)

    //
    // Bit masks for all fields in the ctrl byte
    //
    enum CtrlFlags {
        CTRL_DUTY      = 0x03, // duty setting bits 0-1 (channels 1,2)
        CTRL_SET_WAVE  = 0x03, // wave set flag bits 0-1 (channel 3)
        CTRL_SET_NOISE = 0x01, // noise set flag bit 0 (channel 4)
        CTRL_SET_DUTY  = 0x04, // duty set flag bit 2 (channels 1,2)
        CTRL_SET_ENV   = 0x18, // envelope set flag bits 3-4 (channels 1,2,4)
        CTRL_PANNING   = 0x60, // panning setting bits 5-6 (all channels)
        CTRL_INIT      = 0x80, // init flag bit 7 (all channels)
    };

    enum SettingsFlags {
        SETTINGS_SET_SWEEP = 0x80, // sweep set flag bit 7 (channel 1 only)
    };

    //
    // Panning control flags
    //
    enum Panning {
        PANNING_NOSET = 0x00, // panning remains unchanged
        PANNING_LEFT  = 0x20, // channel panning is set to left
        PANNING_RIGHT = 0x40, // channel panning is set to right
        PANNING_BOTH  = 0x60, // channel panning is set to left and right
    };

    //
    // Duty setting values
    //
    enum Duty {
        DUTY_125      = 0x00,
        DUTY_25       = 0x01,
        DUTY_50       = 0x02,
        DUTY_75       = 0x03
    };

    //
    // Envelope control flags, determines if the envelope will be set
    //
    enum EnvCtrl {
        ENV_NOSET    = 0x00, // envelope remains unchanged
        ENV_SETVOL   = 0x08, // envelope is set with volume in instruction
        ENV_SETNOVOL = 0x10  // envelope is set with volume from note
    };

    enum WaveCtrl {
        WAVE_NOSET   = 0x00, // do not set wave ram
        WAVE_SET     = 0x01, // set wave ram
        WAVE_SETLONG = 0x02  // long play wave
    };

    enum WaveVol {
        WAVE_VOL_MUTE    = 0x00,
        WAVE_VOL_FULL    = 0x01,
        WAVE_VOL_HALF    = 0x02,
        WAVE_VOL_QUARTER = 0x03
    };
};


class WaveTable {

public:
    WaveTable();

    uint8_t* getWave(uint8_t waveId);

    void setWave(uint8_t waveId, uint8_t waveform[WaveChannel::WAVE_RAMSIZE]);

    std::vector<uint8_t>& table();

private:

    std::vector<uint8_t> mTable;

};


class Instrument {

    uint8_t id;
    TrackId trackId;
    // sequence of instructions, one instruction per frame
    std::vector<Instruction> program;

public:

    Instrument(uint8_t id, TrackId track);

    uint8_t getId();
    TrackId getTrackId();
    std::vector<Instruction>& getProgram();
};


class InstrumentRuntime {

    std::vector<Instruction> *program;
    uint8_t fc;
    uint8_t pc;

protected:
    const TrackId trackId;

    InstrumentRuntime(TrackId trackId);

    virtual void execute(Synth &synth, Instruction inst) = 0;

public:

    // returns true if pc is at end of program
    bool isFinished();

    void reset();

    void setProgram(std::vector<Instruction> *program);

    void step(Synth &synth);

};

class PulseInstrumentRuntime : public InstrumentRuntime {

protected:

    void execute(Synth &synth, Instruction inst) override;

public:
    PulseInstrumentRuntime(bool sweep = false);
};


class WaveInstrumentRuntime : public InstrumentRuntime {

protected:

    void execute(Synth &synth, Instruction inst) override;

public:
    WaveInstrumentRuntime();

};


class NoiseInstrumentRuntime : public InstrumentRuntime {

protected:

    void execute(Synth &synth, Instruction inst) override;

public:
    NoiseInstrumentRuntime();

};

Instruction parse(TrackId track, std::string line);


}