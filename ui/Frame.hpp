
#pragma once

#include <memory>


//
// Informational class for a rendered frame. Contains the rendered audio, position
// in the pattern, and so on. The GUI thread uses this information to update visualizers,
// the statusbar, and the current pattern row if follow-mode is enabled.
//
class Frame {

public:
    Frame();
    ~Frame();

    void setSamples(float[] buf, size_t nsamples);

private:
    std::unique_ptr<float[]> mSamples;

};
