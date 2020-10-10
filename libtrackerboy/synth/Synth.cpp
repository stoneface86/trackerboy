
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include "Blip_Buffer.h"

#include <algorithm>
#include <climits>
#include <cmath>


namespace trackerboy {

// PIMPL idiom - this way we can keep Blip_Buffer out of the public API
struct Synth::Internal {

    // blip_buffer
    blargg::Blip_Buffer bbuf;
    // CH1 + CH2
    blargg::Blip_Synth<blargg::blip_good_quality, 30> bsynth1;
    // CH3 + CH4 (lower quality since these channels have a lot of transitions)
    blargg::Blip_Synth<blargg::blip_med_quality, 30> bsynth2;


    Internal() :
        bbuf(),
        bsynth1(),
        bsynth2()

    {
        bsynth1.output(&bbuf);
        bsynth2.output(&bbuf);
    }

    Internal(const Internal &ref) = delete;
    Internal& operator=(const Internal &ref) = delete;

};


Synth::Synth(unsigned samplingRate, float framerate) noexcept :
    mInternal(new Internal()),
    mSamplerate(samplingRate),
    mFramerate(framerate),
    mHf(),
    mSequencer(mHf),
    mCyclesPerFrame(Gbs::CLOCK_SPEED / framerate),
    mCycleOffset(0.0f),
    mFrameBuf(),
    mOutputStat(Gbs::OUT_ALL),
    mChPrev{0},
    mFillPos(0),
    mLastFrameSize(0)
{
    setupBuffers();
}

// required for std::unique_ptr<Internal>
Synth::~Synth() = default;

int16_t* Synth::buffer() noexcept {
    return mFrameBuf.data();
}

HardwareFile& Synth::hardware() noexcept {
    return mHf;
}

// This method is only used by demo and should be removed at some point
// everywhere else uses the frame buffer
void Synth::fill(int16_t buf[], size_t nsamples) noexcept {
    int16_t *dest = buf;

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

size_t Synth::run() noexcept {
    auto intern = mInternal.get();
    auto &bbuf = intern->bbuf;
    auto &bsynth1 = intern->bsynth1;
    auto &bsynth2 = intern->bsynth2;


    // determine number of cycles to run
    float cycles = mCyclesPerFrame + mCycleOffset;
    float wholeCycles;
    mCycleOffset = modff(cycles, &wholeCycles);

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
        // it's named fence as an analogy to memory fences/barriers
        uint32_t fence = std::min(remaining, mSequencer.fence());
        updateOutput<ChType::ch1>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch2>(leftdelta, rightdelta, fence);

        // don't bother calling offset with delta = 0
        if (leftdelta) {
            bsynth1.offset_inline(cycletime, leftdelta, blargg::blip_term_left);
            leftdelta = 0;
        }
        if (rightdelta) {
            bsynth1.offset_inline(cycletime, rightdelta, blargg::blip_term_right);
            rightdelta = 0;
        }

        updateOutput<ChType::ch3>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch4>(leftdelta, rightdelta, fence);
        if (leftdelta)
            bsynth2.offset_inline(cycletime, leftdelta, blargg::blip_term_left);
        if (rightdelta)
            bsynth2.offset_inline(cycletime, rightdelta, blargg::blip_term_right);


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

    // end the frame and copy samples to the synth's frame buffer
    bbuf.end_frame(cycletime);
    auto samples = bbuf.samples_avail();
    bbuf.read_samples(mFrameBuf.data(), samples);
    
    return samples;

}

uint8_t Synth::readRegister(uint16_t addr) const noexcept {
    
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


void Synth::reset() noexcept {
    mCycleOffset = 0.0f;
    std::fill_n(mChPrev, 8, static_cast<uint8_t>(0));
    mFillPos = 0;
    mLastFrameSize = 0;
    // reset hardware components
    mSequencer.reset();
    mHf.env1.reset();
    mHf.env2.reset();
    mHf.env4.reset();
    mHf.sweep1.reset();
    mHf.gen1.reset();
    mHf.gen2.reset();
    mHf.gen3.reset();
    mHf.gen4.reset();

    mInternal->bbuf.clear();
}


void Synth::restart(ChType ch) noexcept {
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

void Synth::setFramerate(float framerate) {
    mFramerate = framerate;
}

void Synth::setFrequency(ChType ch, uint16_t freq) {
    switch (ch) {
        case ChType::ch1:
            mHf.gen1.setFrequency(freq);
            break;
        case ChType::ch2:
            mHf.gen2.setFrequency(freq);
            break;
        case ChType::ch3:
            mHf.gen3.setFrequency(freq);
            break;
        case ChType::ch4:
            break;
    }
}

void Synth::setSamplingRate(unsigned samplingRate) {
    mSamplerate = samplingRate;
}

void Synth::setupBuffers() {
    mCyclesPerFrame = Gbs::CLOCK_SPEED / mFramerate;
    unsigned blipbufsize = static_cast<unsigned>(ceilf(1000.0f / mFramerate));
    mFrameBuf.resize((static_cast<size_t>(mSamplerate / mFramerate) + 1) * 2);

    auto &bbuf = mInternal->bbuf;
    auto &bsynth1 = mInternal->bsynth1;
    auto &bsynth2 = mInternal->bsynth2;
    
    bbuf.set_sample_rate(mSamplerate, blipbufsize);
    bbuf.clock_rate(Gbs::CLOCK_SPEED);
    bbuf.bass_freq(16);

    blargg::blip_eq_t eq(-8.0);
    bsynth1.treble_eq(eq);
    bsynth1.volume(0.5);
    bsynth2.treble_eq(eq);
    bsynth2.volume(0.5);
    reset();
}

void Synth::setOutputEnable(Gbs::OutputFlags flags) noexcept {
    mOutputStat = flags;
}

void Synth::setOutputEnable(ChType ch, Gbs::Terminal term, bool enabled) noexcept {
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

void Synth::writeRegister(uint16_t addr, uint8_t value) noexcept {

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
void Synth::updateOutput(int8_t &leftdelta, int8_t &rightdelta, uint32_t &fence) noexcept {
    
    // get our generator
    // hopefully the compiler removes dead code, if constexpr might be needed instead
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
        // generator output for all channels except 3 is a mask, 0 for off, 0xFF for on
        // AND it with the envelope value to get the current volume
        output &= mask;
    }
    
    // get the previous volumes
    constexpr size_t chint = static_cast<size_t>(ch);
    uint8_t &prevL = mChPrev[chint];
    uint8_t &prevR = mChPrev[chint + 4];

    // convert output stat (NR51) to a mask
    uint8_t maskL = ~((mOutputStat >> chint) & 1) + 1;
    uint8_t maskR = ~((mOutputStat >> (chint + 4)) & 1) + 1;

    uint8_t outputL = output & maskL;
    uint8_t outputR = output & maskR;
    // calculate and accumulate deltas
    leftdelta += static_cast<int8_t>(outputL) - static_cast<int8_t>(prevL);
    rightdelta += static_cast<int8_t>(outputR) - static_cast<int8_t>(prevR);

    // save the previous values for next time
    prevL = outputL;
    prevR = outputR;

    // update the fence (the smallest one will be stepped to)
    fence = std::min(fence, gen->fence());
}

}
