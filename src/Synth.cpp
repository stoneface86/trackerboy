
#include "gbsynth.h"

namespace gbsynth {


    Synth::Synth(float samplingRate) :
        cf(),
        sweep(cf.ch1),
        mixer(cf),
        sequencer(cf, sweep)
    {
        // TODO
    }

    Sweep& Synth::getSweep() {
        return sweep;
    }

    ChannelFile& Synth::getChannels() {
        return cf;
    }

    Mixer& Synth::getMixer() {
        return mixer;
    }

    Sequencer& Synth::getSequencer() {
        return sequencer;
    }

    void Synth::fill(float leftBuf[], float rightBuf[], size_t nsamples) {
        // TODO
    }

}