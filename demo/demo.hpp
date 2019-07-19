
#pragma once

#include "gbsynth.hpp"


class Demo {

protected:
    gbsynth::Synth &synth;
    unsigned counter;

public:
    Demo(gbsynth::Synth &synth);

    virtual void init() = 0;

    // synthesizer is updated for the next run of the demo, if exists
    // the runtime, in milleseconds, is returned. If 0 is returned, then
    // there is no next run.
    virtual long setupNextRun() = 0;

};

class DutyDemo : public Demo {

public:
    void init() override;
    long setupNextRun() override;

};

class SweepDemo : public Demo {

public:
    void init() override;
    long setupNextRun() override;

};