/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "trackerboy/ChType.hpp"

#include <cstddef>
#include <memory>

namespace trackerboy {


class Mixer {

public:

    enum class Pan {
        none = 0,
        left = 1,
        right = 2,
        both = 3
    };

    Mixer(float samplingRate) noexcept;

    //
    // Prepares the frame buffer for mixing
    //
    void beginFrame(float buf[], size_t bufsize) noexcept;

    //
    // Finish sampling the buffer by integrating (running sum).
    //
    void endFrame() noexcept;

    template <Pan pan>
    void addStep(float step, float time) noexcept;

    void reset() noexcept;

    void setSamplingRate(float samplingRate) noexcept;
    

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
