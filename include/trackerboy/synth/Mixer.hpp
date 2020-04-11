
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "trackerboy/ChType.hpp"

namespace trackerboy {


class Mixer {

public:

    enum class Pan {
        none = 0,
        left = 1,
        right = 2,
        both = 3
    };

    Mixer(float samplingRate);

    //
    // Prepares the frame buffer for mixing
    //
    void beginFrame(float buf[], size_t bufsize);

    //
    // Finish sampling the buffer by integrating (running sum).
    //
    void endFrame();

    template <Pan pan>
    void addStep(float step, float time);

    void reset();

    void setSamplingRate(float samplingRate);
    

private:

    static constexpr size_t STEP_PHASES = 32;
    static constexpr size_t STEP_WIDTH = 16;
    static constexpr size_t STEP_CENTER = STEP_WIDTH / 2;

    std::unique_ptr<float[]> mStepTable;

    static constexpr size_t FUTURE_SIZE = (STEP_WIDTH - 1) * 2;
    float mFuture[FUTURE_SIZE];

    // previous samples from integration
    float mPreviousL; // left
    float mPreviousR; // right

    float* mBuf;
    size_t mBufsize;
};

}
