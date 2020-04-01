
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include <algorithm>
#include <climits>
#include <cmath>

namespace {

constexpr unsigned CYCLES_PER_TRIGGER = 8192;

// largest delta = 60 (0xF * 4), so the maximum volume is 15/16 (-0.56 dB)
constexpr float GAIN = 1.0f / 64.0f; 

}


namespace trackerboy {


Synth::Synth(float samplingRate, float framerate) :
    mSamplingRate(samplingRate),
    mFramerate(framerate),
    mHf(),
    mMixer(samplingRate),
    mSequencer(mHf),
    mCyclesPerSample(Gbs::CLOCK_SPEED / samplingRate),
    mCyclesPerFrame(Gbs::CLOCK_SPEED / framerate),
    mCycleOffset(0.0f),
    mSampleOffset(0.0f),
    mFrameBuf((static_cast<size_t>(samplingRate / framerate) + 1) * 2),
    mOutputStat(Gbs::OUT_ALL),
    mChPrev{0},
    mFillPos(0),
    mLastFrameSize(0)
{
    // NOTE: mTriggerTimings will need to be recalculated if the sampling rate
    // changes. Currently there is no way to change it after construction,
    // although this may change in the future.
}

float* Synth::buffer() {
    return mFrameBuf.data();
}

HardwareFile& Synth::hardware() {
    return mHf;
}

void Synth::fill(float buf[], size_t nsamples) {
    float *dest = buf;

    while (nsamples) {
        if (mFillPos == mLastFrameSize) {
            mLastFrameSize = run();
            mFillPos = 0;
        }

        size_t samples = std::min(mLastFrameSize - mFillPos, nsamples);
        size_t amount = samples * 2; // stereo, so 2 values per sample
        std::copy_n(mFrameBuf.begin() + (mFillPos * 2), amount, dest);
        dest += amount;

        nsamples -= samples;
        mFillPos += samples;
    }
}

size_t Synth::run() {
    // determine number of cycles to run
    float cycles = mCyclesPerFrame + mCycleOffset;
    float wholeCycles;
    mCycleOffset = modff(cycles, &wholeCycles);

    // determine number of samples needed
    float samples = (mCyclesPerFrame / mCyclesPerSample) + mSampleOffset;
    float wholeSamples;
    mSampleOffset = modff(samples, &wholeSamples);
    size_t nsamples = static_cast<size_t>(wholeSamples);

    // setup the buffer
    mMixer.beginFrame(mFrameBuf.data(), nsamples);

    uint32_t remaining = static_cast<uint32_t>(wholeCycles);
    uint32_t cycletime = 0;
    while (remaining) {
        // delta is the amount of change from the previous output
        // 0 indicates no change, or no transition
        int8_t leftdelta = 0;
        int8_t rightdelta = 0;

        // keep track of the smallest fence
        // a fence is the minimum number of cycles until state changes
        // ie generator output changing, envelope volume decreasing, etc
        uint32_t fence = std::min(remaining, mSequencer.fence());
        updateOutput<ChType::ch1>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch2>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch3>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch4>(leftdelta, rightdelta, fence);

        if (leftdelta || rightdelta) {
            // convert time in cycles to time in samples
            float sampletime = cycletime / mCyclesPerSample;
            if (leftdelta == rightdelta) {
                // since both deltas are the same, we can add the step to both terminals
                mMixer.addStep<Mixer::Pan::both>(leftdelta * GAIN, sampletime);
            } else {
                if (leftdelta) {
                    mMixer.addStep<Mixer::Pan::left>(leftdelta * GAIN, sampletime);
                }
                if (rightdelta) {
                    mMixer.addStep<Mixer::Pan::right>(rightdelta * GAIN, sampletime);
                }
            }
        }

        // run all components to the smallest fence
        mSequencer.step(fence);
        mHf.gen1.step(fence);
        mHf.gen2.step(fence);
        mHf.gen3.step(fence);
        mHf.gen4.step(fence);

        // update cycle counters
        remaining -= fence;
        cycletime += fence;
    }

    // we are done adding steps, end the frame
    mMixer.endFrame();
    return nsamples;

}

uint8_t Synth::readRegister(uint16_t addr) {
    
    /*
    Read masks

         NRx0 NRx1 NRx2 NRx3 NRx4
        ---------------------------
    NR1x  $80  $3F $00  $FF  $BF 
    NR2x  $FF  $3F $00  $FF  $BF 
    NR3x  $7F  $FF $9F  $FF  $BF 
    NR4x  $FF  $FF $00  $00  $BF 
    NR5x  $00  $00 $70

    $FF27-$FF2F always read back as $FF
    */

    #define readDuty(gen) (0x3F | (gen.duty() << 6))

    switch (addr) {
        case Gbs::REG_NR10:
            return mHf.sweep1.readRegister();
        case Gbs::REG_NR11:
            return readDuty(mHf.gen1);
        case Gbs::REG_NR12:
            return mHf.env1.readRegister();
        case Gbs::REG_NR13:
            return 0xFF;
        case Gbs::REG_NR14:
            return 0xBF;
        case Gbs::REG_NR21:
            return readDuty(mHf.gen2);
        case Gbs::REG_NR22:
            return mHf.env2.readRegister();
        case Gbs::REG_NR23:
            return 0xFF;
        case Gbs::REG_NR24:
            return 0xBF;
        case Gbs::REG_NR30:
            // TODO
            return 0x7F;
        case Gbs::REG_NR31:
            return 0xFF;
        case Gbs::REG_NR32:
            return 0x9F | (mHf.gen3.volume());
        case Gbs::REG_NR33:
            return 0xFF;
        case Gbs::REG_NR34:
            return 0xBF;
        case Gbs::REG_NR41:
            return 0xBF;
        case Gbs::REG_NR42:
            return mHf.env4.readRegister();
        case Gbs::REG_NR43:
            return mHf.gen4.readRegister();
        case Gbs::REG_NR44:
            return 0xBF;
        case Gbs::REG_NR50:
            return 0;
        case Gbs::REG_NR51:
            return mOutputStat;
        case Gbs::REG_NR52:
            return 0x70;
        default:
            return 0xFF;


    }
}


void Synth::reset() {
    // TODO: implement later
}


void Synth::restart(ChType ch) {
    switch (ch) {
        case ChType::ch1:
            mHf.env1.restart();
            mHf.sweep1.restart();
            mHf.gen1.restart();
            break;
        case ChType::ch2:
            mHf.env2.restart();
            mHf.gen2.restart();
            break;
        case ChType::ch3:
            mHf.gen3.restart();
            break;
        case ChType::ch4:
            mHf.env4.restart();
            mHf.gen4.restart();
            break;
    }
}


void Synth::setOutputEnable(Gbs::OutputFlags flags) {
    mOutputStat = flags;
}

void Synth::setOutputEnable(ChType ch, Gbs::Terminal term, bool enabled) {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    if (enabled) {
        mOutputStat |= flag;
    } else {
        mOutputStat &= ~flag;
    }
}

void Synth::writeRegister(uint16_t addr, uint8_t value) {

    #define writeDuty(gen) gen.setDuty(static_cast<Gbs::Duty>(value >> 6))
    #define writeFreqLSB(gen) gen.setFrequency((gen.frequency() & 0xF0) | value)
    #define writeFreqMSB(gen) gen.setFrequency((gen.frequency() & 0x0F) | (value << 8))
    #define trigger(ch) if (value & 0x80) restart(ch)

    switch (addr) {
        case Gbs::REG_NR10:
            mHf.sweep1.writeRegister(value);
            break;
        case Gbs::REG_NR11:
            writeDuty(mHf.gen1);
            // length counters aren't implemented so ignore the other 6 bits
            break;
        case Gbs::REG_NR12:
            mHf.env1.writeRegister(value);
            break;
        case Gbs::REG_NR13:
            writeFreqLSB(mHf.gen1);
            break;
        case Gbs::REG_NR14:
            writeFreqMSB(mHf.gen1);
            trigger(ChType::ch1);
            break;
        case Gbs::REG_NR21:
            writeDuty(mHf.gen2);
            break;
        case Gbs::REG_NR22:
            mHf.env2.writeRegister(value);
            break;
        case Gbs::REG_NR23:
            writeFreqLSB(mHf.gen2);
            break;
        case Gbs::REG_NR24:
            writeFreqMSB(mHf.gen2);
            trigger(ChType::ch2);
            break;
        case Gbs::REG_NR30:
            // TODO: implement functionality in WaveGen
            break;
        case Gbs::REG_NR31:
            // no length counter so do nothing
            break;
        case Gbs::REG_NR32:
            mHf.gen3.setVolume(static_cast<Gbs::WaveVolume>((value >> 5) & 0x3));
            break;
        case Gbs::REG_NR33:
            writeFreqLSB(mHf.gen3);
            break;
        case Gbs::REG_NR34:
            writeFreqMSB(mHf.gen3);
            trigger(ChType::ch3);
            break;
        case Gbs::REG_NR41:
            // no length counter do nothing
            break;
        case Gbs::REG_NR42:
            mHf.env4.writeRegister(value);
            break;
        case Gbs::REG_NR43:
            mHf.gen4.writeRegister(value);
            break;
        case Gbs::REG_NR44:
            trigger(ChType::ch4);
            break;
        case Gbs::REG_NR50:
            // not implemented, do nothing
            break;
        case Gbs::REG_NR51:
            mOutputStat = value;
            break;
        case Gbs::REG_NR52:
            // not implemented
            break;
        default:
            break;
    }
}


template <ChType ch>
void Synth::updateOutput(int8_t &leftdelta, int8_t &rightdelta, uint32_t &fence) {
    Generator *gen = nullptr;
    uint8_t mask = 0xF;
    switch (ch) {
        case ChType::ch1:
            gen = &mHf.gen1;
            mask = mHf.env1.value();
            break;
        case ChType::ch2:
            gen = &mHf.gen2;
            mask = mHf.env2.value();
            break;
        case ChType::ch3:
            gen = &mHf.gen3;
            break;
        case ChType::ch4:
            gen = &mHf.gen4;
            mask = mHf.env4.value();
            break;

    }

    uint8_t output = gen->output();
    if constexpr (ch != ChType::ch3) {
        output &= mask;
    }
    

    constexpr size_t chint = static_cast<size_t>(ch);
    uint8_t &prevL = mChPrev[chint];
    uint8_t &prevR = mChPrev[chint + 4];

    uint8_t maskL = ~((mOutputStat >> chint) & 1) + 1;
    uint8_t maskR = ~((mOutputStat >> (chint + 4)) & 1) + 1;

    uint8_t outputL = output & maskL;
    uint8_t outputR = output & maskR;
    int8_t deltaL = static_cast<int8_t>(outputL) - static_cast<int8_t>(prevL);
    int8_t deltaR = static_cast<int8_t>(outputR) - static_cast<int8_t>(prevR);

    leftdelta += deltaL;
    rightdelta += deltaR;



    prevL = outputL;
    prevR = outputR;
    fence = std::min(fence, gen->fence());
}

}
