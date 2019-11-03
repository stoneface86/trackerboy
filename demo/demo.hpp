
#pragma once

#include <string>

#include "trackerboy/synth/Synth.hpp"


class Demo {

protected:
    std::string name;

public:

    Demo(std::string name);

    std::string getName();

    virtual void init(trackerboy::Synth &synth) = 0;

    // synthesizer is updated for the next run of the demo, if exists
    // the runtime, in milleseconds, is returned. If 0 is returned, then
    // there is no next run.
    virtual long setupNextRun(trackerboy::Synth &synth, unsigned counter) = 0;

};

class DutyDemo : public Demo {

public:
    DutyDemo();

    void init(trackerboy::Synth &synth) override;
    long setupNextRun(trackerboy::Synth &synth, unsigned counter) override;

};

class SweepDemo : public Demo {

public:
    SweepDemo();

    void init(trackerboy::Synth &synth) override;
    long setupNextRun(trackerboy::Synth &synth, unsigned counter) override;

};

class WaveDemo : public Demo {

    uint8_t wavedata[trackerboy::Gbs::WAVE_RAMSIZE];

public:
    WaveDemo(const uint8_t wavedata[trackerboy::Gbs::WAVE_RAMSIZE], std::string waveName);

    void init(trackerboy::Synth &synth) override;
    long setupNextRun(trackerboy::Synth &synth, unsigned counter) override;

};

class WaveVolDemo : public Demo {

public:
    WaveVolDemo();

    void init(trackerboy::Synth& synth) override;
    long setupNextRun(trackerboy::Synth& synth, unsigned counter) override;

};

extern const uint8_t WAVEDATA_TRIANGLE[trackerboy::Gbs::WAVE_RAMSIZE];
extern const uint8_t WAVEDATA_SAWTOOTH[trackerboy::Gbs::WAVE_RAMSIZE];
extern const uint8_t WAVEDATA_SINE[trackerboy::Gbs::WAVE_RAMSIZE];
