
#pragma once

#include "gbsynth.hpp"


class Demo {

public:

    virtual void init(gbsynth::Synth &synth) = 0;

    // synthesizer is updated for the next run of the demo, if exists
    // the runtime, in milleseconds, is returned. If 0 is returned, then
    // there is no next run.
    virtual long setupNextRun(gbsynth::Synth &synth, unsigned counter) = 0;

};

class DutyDemo : public Demo {

public:
    void init(gbsynth::Synth &synth) override;
    long setupNextRun(gbsynth::Synth &synth, unsigned counter) override;

};

class SweepDemo : public Demo {

public:
    void init(gbsynth::Synth &synth) override;
    long setupNextRun(gbsynth::Synth &synth, unsigned counter) override;

};

class WaveDemo : public Demo {

    uint8_t wavedata[gbsynth::WAVE_RAMSIZE];

public:
    WaveDemo(const uint8_t wavedata[gbsynth::WAVE_RAMSIZE]);

    void init(gbsynth::Synth &synth) override;
    long setupNextRun(gbsynth::Synth &synth, unsigned counter) override;

};

extern const uint8_t WAVEDATA_TRIANGLE[gbsynth::WAVE_RAMSIZE];
extern const uint8_t WAVEDATA_SAWTOOTH[gbsynth::WAVE_RAMSIZE];
extern const uint8_t WAVEDATA_SINE[gbsynth::WAVE_RAMSIZE];
