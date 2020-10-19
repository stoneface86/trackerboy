
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include "synth/Sequencer.hpp"
#include "synth/HardwareFile.hpp"

#include "Blip_Buffer.h"

#include <algorithm>
#include <climits>
#include <cmath>


// number of volume units for each blip synth that are unused
//constexpr int HEADROOM = 4;
constexpr double HEADROOM = 0.7079457844; // -3 dB

// simulate cycle delays when reading/writing registers
// ldh a, [n16]
constexpr uint32_t CYCLES_PER_READ = 3;
// ldh [n16], a
constexpr uint32_t CYCLES_PER_WRITE = 3;

namespace trackerboy {

// PIMPL idiom - this way we can keep Blip_Buffer out of the public API
struct Synth::Internal {

    // blip_buffer
    blargg::Blip_Buffer bbuf;
    // each channel oscillates between -15 to 15 (or -F to F), silent at 0
    // thus a channel's range is 30 volume units (15 - (-15) = 30)

    // CH1 + CH2
    blargg::Blip_Synth<blargg::blip_good_quality, 60> bsynth1;
    // CH3 + CH4 (lower quality since these channels have a lot of transitions)
    blargg::Blip_Synth<blargg::blip_med_quality, 60> bsynth2;

    // Hardware components
    HardwareFile hf;
    Sequencer sequencer;


    Internal() :
        bbuf(),
        bsynth1(),
        bsynth2(),
        hf(),
        sequencer(hf)

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
    mCyclesPerFrame(Gbs::CLOCK_SPEED / framerate),
    mCycleOffset(0.0f),
    mFrameBuf(),
    mOutputStat(Gbs::OUT_ALL),
    mChPrev{0},
    mLastFrameSize(0),
    mCycletime(0)
{
    setupBuffers();
}

// required for std::unique_ptr<Internal>
Synth::~Synth() = default;

int16_t* Synth::buffer() noexcept {
    return mFrameBuf.data();
}

size_t Synth::run() noexcept {

    // determine number of cycles to run for the next frame
    float cycles = mCyclesPerFrame + mCycleOffset;
    float wholeCycles;
    mCycleOffset = modff(cycles, &wholeCycles);

    // step to the end of the frame
    step(static_cast<uint32_t>(wholeCycles) - mCycletime);

    // end the frame and copy samples to the synth's frame buffer
    auto &bbuf = mInternal->bbuf;
    bbuf.end_frame(mCycletime);
    auto samples = bbuf.samples_avail();
    bbuf.read_samples(mFrameBuf.data(), samples);
    
    // reset time offset
    mCycletime = 0;

    mLastFrameSize = samples;
    return samples;

}

void Synth::step(uint32_t cycles) noexcept {

    auto &bsynth1 = mInternal->bsynth1;
    auto &bsynth2 = mInternal->bsynth2;
    auto &sequencer = mInternal->sequencer;
    auto &hf = mInternal->hf;

    while (cycles) {

        // delta is the amount of change from the previous output
        // 0 indicates no change, or no transition
        int8_t leftdelta = 0;
        int8_t rightdelta = 0;

        // keep track of the smallest fence
        // a fence is the minimum number of cycles until state changes
        // ie generator output changing, envelope volume decreasing, etc
        // it's named fence as an analogy to memory fences/barriers
        uint32_t fence = std::min(cycles, sequencer.fence());
        updateOutput<ChType::ch1>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch2>(leftdelta, rightdelta, fence);

        // don't bother calling offset with delta = 0
        if (leftdelta) {
            bsynth1.offset_inline(mCycletime, leftdelta, blargg::blip_term_left);
            leftdelta = 0;
        }
        if (rightdelta) {
            bsynth1.offset_inline(mCycletime, rightdelta, blargg::blip_term_right);
            rightdelta = 0;
        }

        updateOutput<ChType::ch3>(leftdelta, rightdelta, fence);
        updateOutput<ChType::ch4>(leftdelta, rightdelta, fence);
        if (leftdelta)
            bsynth2.offset_inline(mCycletime, leftdelta, blargg::blip_term_left);
        if (rightdelta)
            bsynth2.offset_inline(mCycletime, rightdelta, blargg::blip_term_right);



        uint32_t cyclesToStep = std::min(cycles, fence);
        // run all components to the smallest fence
        sequencer.step(cyclesToStep);
        hf.gen1.step(cyclesToStep);
        hf.gen2.step(cyclesToStep);
        hf.gen3.step(cyclesToStep);
        hf.gen4.step(cyclesToStep);

        // update cycle counters
        cycles -= cyclesToStep;
        mCycletime += cyclesToStep;
    }

}

uint8_t Synth::readRegister(uint16_t addr) noexcept {
    step(CYCLES_PER_READ);

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

    auto &hf = mInternal->hf;

    switch (addr) {
        case Gbs::REG_NR10:
            return hf.sweep1.readRegister();
        case Gbs::REG_NR11:
            return readDuty(hf.gen1);
        case Gbs::REG_NR12:
            return hf.env1.readRegister();
        case Gbs::REG_NR13:
            return 0xFF;
        case Gbs::REG_NR14:
            return 0xBF;
        case Gbs::REG_NR21:
            return readDuty(hf.gen2);
        case Gbs::REG_NR22:
            return hf.env2.readRegister();
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
            return 0x9F | (hf.gen3.volume());
        case Gbs::REG_NR33:
            return 0xFF;
        case Gbs::REG_NR34:
            return 0xBF;
        case Gbs::REG_NR41:
            return 0xBF;
        case Gbs::REG_NR42:
            return hf.env4.readRegister();
        case Gbs::REG_NR43:
            return hf.gen4.readRegister();
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
    mCycletime = 0;
    std::fill_n(mChPrev, 8, static_cast<int8_t>(0));
    mLastFrameSize = 0;
    // reset hardware components
    mInternal->sequencer.reset();
    auto &hf = mInternal->hf;
    hf.env1.reset();
    hf.env2.reset();
    hf.env4.reset();
    hf.sweep1.reset();
    hf.gen1.reset();
    hf.gen2.reset();
    hf.gen3.reset();
    hf.gen4.reset();

    mInternal->bbuf.clear();
}

void Synth::setFramerate(float framerate) {
    mFramerate = framerate;
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
    bbuf.bass_freq(20);

    blargg::blip_eq_t eq(-24.0, 12000, mSamplerate);
    bsynth1.treble_eq(eq);
    bsynth1.volume(0.5 * HEADROOM);
    bsynth2.treble_eq(eq);
    bsynth2.volume(0.5 * HEADROOM);
    reset();
}

void Synth::setOutputEnable(ChType ch, Gbs::Terminal term, bool enabled) noexcept {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    uint8_t outputstat = readRegister(Gbs::REG_NR51);
    if (enabled) {
        outputstat |= flag;
    } else {
        outputstat &= ~flag;
    }

    writeRegister(Gbs::REG_NR51, outputstat);
}

void Synth::setWaveram(Waveform &waveform) {
    // turn CH3 DAC off
    writeRegister(Gbs::REG_NR30, 0);

    // write the waveform
    mInternal->hf.gen3.copyWave(waveform);

    // simulate writing the waveform via:
    // ; hl points to the waveform to copy
    // WAVE_POS = 0
    // REPT 16
    //     ld   a, [hl+]                ; 2 cycles
    //     ldh  [$FF30 + WAVE_POS], a   ; 3 cycles
    // WAVE_POS = WAVE_POS + 1
    // ENDR                             ; 5 * 16 = 80 cycles
    step(80);

    // turn DAC back on
    writeRegister(Gbs::REG_NR30, 0x80);
}

void Synth::writeRegister(uint16_t addr, uint8_t value) noexcept {

    // run the synth for a few cycles, changes occurring at the end
    step(CYCLES_PER_WRITE);

    #define writeDuty(gen) gen.setDuty(static_cast<Gbs::Duty>(value >> 6))
    #define writeFreqLSB(gen) gen.setFrequency((gen.frequency() & 0xFF00) | value)
    #define writeFreqMSB(gen) gen.setFrequency((gen.frequency() & 0x00FF) | (value << 8))
    #define onTrigger() if (!!(value & 0x80))

    auto &hf = mInternal->hf;
    switch (addr) {
        case Gbs::REG_NR10:
            hf.sweep1.writeRegister(value);
            break;
        case Gbs::REG_NR11:
            writeDuty(hf.gen1);
            // length counters aren't implemented so ignore the other 6 bits
            break;
        case Gbs::REG_NR12:
            hf.env1.writeRegister(value);
            break;
        case Gbs::REG_NR13:
            writeFreqLSB(hf.gen1);
            break;
        case Gbs::REG_NR14:
            writeFreqMSB(hf.gen1);
            onTrigger() {
                hf.env1.restart();
                hf.sweep1.restart();
                hf.gen1.restart();
            }
            break;
        case Gbs::REG_NR21:
            writeDuty(hf.gen2);
            break;
        case Gbs::REG_NR22:
            hf.env2.writeRegister(value);
            break;
        case Gbs::REG_NR23:
            writeFreqLSB(hf.gen2);
            break;
        case Gbs::REG_NR24:
            writeFreqMSB(hf.gen2);
            onTrigger() {
                hf.env2.restart();
                hf.gen2.restart();
            }
            break;
        case Gbs::REG_NR30:
            // TODO: implement functionality in WaveGen
            break;
        case Gbs::REG_NR31:
            // no length counter so do nothing
            break;
        case Gbs::REG_NR32:
            hf.gen3.setVolume(static_cast<Gbs::WaveVolume>((value >> 5) & 0x3));
            break;
        case Gbs::REG_NR33:
            writeFreqLSB(hf.gen3);
            break;
        case Gbs::REG_NR34:
            writeFreqMSB(hf.gen3);
            onTrigger() {
                hf.gen3.restart();
            }
            break;
        case Gbs::REG_NR41:
            // no length counter do nothing
            break;
        case Gbs::REG_NR42:
            hf.env4.writeRegister(value);
            break;
        case Gbs::REG_NR43:
            hf.gen4.writeRegister(value);
            break;
        case Gbs::REG_NR44:
            onTrigger() {
                hf.env4.restart();
                hf.gen4.restart();
            }
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

    int8_t output;
    int8_t envelope;
    uint32_t thisFence;
    auto &hf = mInternal->hf;

    if constexpr (ch == ChType::ch1) {
        output = hf.gen1.output();
        envelope = hf.env1.value();
        thisFence = hf.gen1.fence();
    } else if constexpr (ch == ChType::ch2) {
        output = hf.gen2.output();
        envelope = hf.env2.value();
        thisFence = hf.gen2.fence();
    } else if constexpr (ch == ChType::ch3) {
        output = hf.gen3.output();
        thisFence = hf.gen3.fence();
    } else {
        output = hf.gen4.output();
        envelope = hf.env4.value();
        thisFence = hf.gen4.fence();
    }

    //uint8_t output = gen->output();
    if constexpr (ch != ChType::ch3) {
        // if generator output is 1, output envelope volume otherwise negative volume
        output = (output) ? envelope : -envelope;
    } else {
        //  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        // -F -D -B -9 -7 -5 -3 -1 +1 +3 +5 +7 +9 +B +D +F
        output = 2 * output - 15;
    }
    
    // get the previous volumes
    constexpr size_t chint = static_cast<size_t>(ch);
    int8_t &prevL = mChPrev[chint];
    int8_t &prevR = mChPrev[chint + 4];

    // convert output stat (NR51) to a mask
    uint8_t maskL = ~((mOutputStat >> chint) & 1) + 1;
    uint8_t maskR = ~((mOutputStat >> (chint + 4)) & 1) + 1;

    int8_t outputL = output & maskL;
    int8_t outputR = output & maskR;
    // calculate and accumulate deltas
    leftdelta += outputL - prevL;
    rightdelta += outputR - prevR;

    // save the previous values for next time
    prevL = outputL;
    prevR = outputR;

    // update the fence (the smallest one will be stepped to)
    fence = std::min(fence, thisFence);
}

}
