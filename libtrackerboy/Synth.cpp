
#include "trackerboy/Synth.hpp"

#include <cmath>


namespace trackerboy {


Synth::Synth(unsigned samplingRate, float framerate) noexcept :
    mBuffer(samplingRate, static_cast<size_t>(samplingRate / framerate) + 1),
    mApu(mBuffer),
    mSamplerate(samplingRate),
    mFramerate(framerate),
    mCyclesPerFrame(gbapu::constants::CLOCK_SPEED<float> / mFramerate),
    mCycleOffset(0.0f),
    mFrameBuf(),
    mLastFrameSize(0),
    mResizeRequired(true)
{
    setupBuffers();
}

gbapu::Apu& Synth::apu() noexcept {
    return mApu;
}

int16_t* Synth::buffer() noexcept {
    return mFrameBuf.data();
}

size_t Synth::framesize() const noexcept {
    return (mFrameBuf.size() / 2) - 1;
}

size_t Synth::run() noexcept {

    // determine number of cycles to run for the next frame
    float cycles = mCyclesPerFrame + mCycleOffset;
    float wholeCycles;
    mCycleOffset = modff(cycles, &wholeCycles);

    // step to the end of the frame
    mApu.stepTo(static_cast<uint32_t>(wholeCycles));
    mApu.endFrame();

    // end the frame and copy samples to the synth's frame buffer
    auto framedata = mFrameBuf.data();

    auto samples = mBuffer.available();
    mBuffer.read(framedata, samples);

    mLastFrameSize = samples;
    return samples;

}


void Synth::reset() noexcept {
    mApu.reset();
    mBuffer.clear();
    mCycleOffset = 0.0f;
    mLastFrameSize = 0;

    // turn sound on
    mApu.writeRegister(gbapu::Apu::REG_NR52, 0x80, 0);
    mApu.writeRegister(gbapu::Apu::REG_NR50, 0x77, 0);
}

void Synth::setFramerate(float framerate) {
    if (mFramerate != framerate) {
        mFramerate = framerate;
        mResizeRequired = true;
    }
}

void Synth::setSamplingRate(unsigned samplingRate) {
    if (mSamplerate != samplingRate) {
        mSamplerate = samplingRate;
        mResizeRequired = true;
    }
}

void Synth::setVolume(int percent) {
    mBuffer.setVolume(percent / 100.0f);
}

void Synth::setupBuffers() {
    if (mResizeRequired) {
        mCyclesPerFrame = gbapu::constants::CLOCK_SPEED<float> / mFramerate;
        size_t samplesPerFrame = static_cast<size_t>(mSamplerate / mFramerate) + 1;
        mFrameBuf.resize(samplesPerFrame * 2);

        mBuffer.setSamplerate(mSamplerate);
        mBuffer.setBuffersize(samplesPerFrame);
        mBuffer.resize();

        reset();
        mResizeRequired = false;
    }

    
}

//void Synth::setWaveram(Waveform &waveform) {
//    // turn CH3 DAC off
//    writeRegister(Gbs::REG_NR30, 0);
//
//    // write the waveform
//    mInternal->hf.gen3.copyWave(waveform);
//
//    // simulate writing the waveform via:
//    // ; hl points to the waveform to copy
//    // WAVE_POS = 0
//    // REPT 16
//    //     ld   a, [hl+]                ; 2 cycles
//    //     ldh  [$FF30 + WAVE_POS], a   ; 3 cycles
//    // WAVE_POS = WAVE_POS + 1
//    // ENDR                             ; 5 * 16 = 80 cycles
//    step(80);
//
//    // turn DAC back on
//    writeRegister(Gbs::REG_NR30, 0x80);
//}

}
