
#include "trackerboy/synth/Mixer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>



namespace trackerboy {

namespace {

constexpr double PI = 3.141592653589793;
constexpr double DOUBLE_PI = 6.283185307179586;

// this is only needed for sigma-approximation
//double sinc(double x) noexcept {
//    if (x == 0.0f) {
//        return 1.0f;
//    } else {
//        double pix = PI * x;
//        return sin(pix) / pix;
//    }
//}

template <Mixer::Pan pan>
inline void mix(float sample, float *&dest) noexcept {
    constexpr int panInt = static_cast<int>(pan);

    if constexpr (!!(panInt & static_cast<int>(Mixer::Pan::left))) {
        *dest++ += sample;
    } else {
        dest++;
    }

    if constexpr (!!(panInt & static_cast<int>(Mixer::Pan::right))) {
        *dest++ += sample;
    } else {
        dest++;
    }
}

}



Mixer::Mixer(float samplingRate) noexcept :
    mStepTable(new float[STEP_PHASES * STEP_WIDTH]),
    mFuture{ 0.0f },
    mPreviousL(0.0f),
    mPreviousR(0.0f),
    mBuf(nullptr),
    mBufsize(0)
{
    setSamplingRate(samplingRate);
}

template <Mixer::Pan pan>
void Mixer::addStep(float step, float time) noexcept {

    float timeWhole;
    float timeFract = modff(time, &timeWhole);
    unsigned phaseIndex = static_cast<unsigned>(timeFract * static_cast<unsigned>(STEP_PHASES));
    float *stepset = mStepTable.get() + (phaseIndex * STEP_WIDTH);

    size_t timeIndex = static_cast<size_t>(timeWhole);
    size_t stepEnd;
    if (timeIndex + STEP_WIDTH > mBufsize) {
        stepEnd = mBufsize - timeIndex;
    } else {
        stepEnd = STEP_WIDTH;
    }

    float *dest = mBuf + (timeIndex * 2);
    float error = step;
    for (size_t i = 0; i != stepEnd; ++i) {
        float sample = step * stepset[i];
        mix<pan>(sample, dest);
        error -= sample;
    }

    dest = mFuture;
    for (size_t i = stepEnd; i != STEP_WIDTH; ++i) {
        float sample = step * stepset[i];
        mix<pan>(sample, dest);
        error -= sample;
    }

    // add error
    size_t center = timeIndex + STEP_CENTER;
    if (center >= mBufsize) {
        dest = mFuture + ((center - mBufsize) * 2);
    } else {
        dest = mBuf + (center * 2);
    }

    mix<pan>(error, dest);

}


void Mixer::beginFrame(float buf[], size_t bufsize) noexcept {
    // copy future steps to the start of the buffer
    std::copy_n(mFuture, FUTURE_SIZE, buf);
    // zero the rest
    std::fill_n(buf + FUTURE_SIZE, (bufsize * 2) - FUTURE_SIZE, 0.0f);
    // clear the future buffer
    std::fill_n(mFuture, FUTURE_SIZE, 0.0f);

    mBuf = buf;
    mBufsize = bufsize;
}

void Mixer::endFrame() noexcept {
    assert(mBuf != nullptr);

    #ifdef _MSC_VER
    // false positive when accessing mBuf
    #pragma warning(push)
    #pragma warning(disable : 28182 )
    #endif

    float *dest = mBuf;
    for (size_t i = 0; i != mBufsize; ++i) {
        *dest += mPreviousL;
        mPreviousL = *dest++;
        *dest += mPreviousR;
        mPreviousR = *dest++;
    }

    #ifdef _MSC_VER
    #pragma warning(pop)
    #endif

}

void Mixer::reset() noexcept {
    // clear previous and future values
    std::fill_n(mFuture, FUTURE_SIZE, 0.0f);
    mPreviousL = 0.0f;
    mPreviousR = 0.0f;
}

void Mixer::setSamplingRate(float samplingRate) noexcept {
    // (re)generate the step table
    constexpr float FREQUENCY = 32.0f;

    const unsigned HARMONICS = static_cast<unsigned>(floorf(samplingRate / (2 * FREQUENCY)));
    const double samplingRateInverted = 1.0 / samplingRate;
    constexpr double AMPLITUDE = 0.5 / 0.7853;

    float *stepset = mStepTable.get();
    for (size_t i = 0; i != STEP_PHASES; ++i) {
        double phase = static_cast<double>(i) / STEP_PHASES * -1.0f;

        // sample the step
        for (size_t j = 0; j != STEP_WIDTH; ++j) {
            int index = static_cast<int>(j) - STEP_CENTER + 1;
            double angle = (static_cast<double>(index) + 0.5 + phase) * samplingRateInverted;
            // start at 0.5 so the wave goes from 0 to 1
            double sample = 0.5f;
            // add the sines of every odd harmonic
            for (unsigned h = 1; h < HARMONICS; h += 2) {
                // lanczos factor, reduces gibbs phenomenon 
                //double lanczos = sinc(static_cast<double>(h) / HARMONICS);
                sample += /*lanczos **/ AMPLITUDE * (sin(h * DOUBLE_PI * angle * FREQUENCY) / h);
            }
            stepset[j] = static_cast<float>(sample);
        }

        // differentiate (first difference)
        float prev = 0.0f; // prev is 0 from the way we setup the step
        float error = 1.0f;
        for (size_t j = 0; j != STEP_WIDTH; ++j) {
            float cur = stepset[j];
            float delta = cur - prev;
            prev = cur;
            stepset[j] = delta;
            error -= delta;
        }

        // add error so that the step sums to 1.0f
        float halferror = error * 0.5f;
        stepset[STEP_CENTER] += halferror;
        stepset[STEP_CENTER - 1] += halferror;

        stepset += STEP_WIDTH;
    }
}


template void Mixer::addStep<Mixer::Pan::left>(float step, float time) noexcept;
template void Mixer::addStep<Mixer::Pan::right>(float step, float time) noexcept;
template void Mixer::addStep<Mixer::Pan::both>(float step, float time) noexcept;



}
