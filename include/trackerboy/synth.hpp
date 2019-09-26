
#pragma once

#include "gbs.hpp"

#include <cstdint>
#include <type_traits>
using std::uint8_t;
using std::uint16_t;

namespace trackerboy {

enum class ChType : uint8_t {
    ch1 = 0,
    ch2 = 1,
    ch3 = 2,
    ch4 = 3
};

class Channel {
    uint8_t lengthCounter;
    bool continuous;
    bool enabled;

protected:
    uint8_t currentSample;
    uint8_t length;
    uint16_t frequency;
    const unsigned freqMultiplier;
    unsigned freqCounter;
    unsigned freqCounterMax;
    
    Channel(bool ch3 = false);

public:

    virtual ~Channel() = default;

    void disable();
    uint8_t getCurrentSample();
    virtual float getCurrentVolume();
    void lengthStep();
    virtual void reset();
    void setContinuousOutput(bool continuous);
    void setFrequency(uint16_t frequency);
    void setLength(uint8_t length);
    virtual void step(unsigned cycles) = 0;
};


class EnvChannel : public Channel {
    uint8_t envCounter;

protected:
    uint8_t envelope;
    Gbs::EnvMode envMode;
    uint8_t envLength;

    EnvChannel();

public:

    virtual ~EnvChannel() = default;

    void envStep();
    float getCurrentVolume() override;
    void reset() override;
    void setEnv(uint8_t envReg);
    void setEnvLength(uint8_t length);
    void setEnvMode(Gbs::EnvMode mode);
    void setEnvStep(uint8_t step);
};


class PulseChannel : public EnvChannel {
    Gbs::Duty duty;
    unsigned dutyCounter;

public:
    
    PulseChannel();

    virtual void reset() override;
    void setDuty(Gbs::Duty duty);
    void step(unsigned cycles) override;

};


class SweepPulseChannel : public PulseChannel {
    Gbs::SweepMode sweepMode;
    uint8_t sweepTime;
    uint8_t sweepShift;

    uint8_t sweepCounter;

public:

    SweepPulseChannel();

    void reset() override;
    void setSweep(uint8_t sweepReg);
    void setSweepMode(Gbs::SweepMode mode);
    void setSweepShift(uint8_t n);
    void setSweepTime(uint8_t ts);
    void sweepStep();
};


class WaveChannel : public Channel {

public:
    static constexpr size_t WAVE_SIZE = 32;
    static constexpr size_t WAVE_RAMSIZE = 16;

    WaveChannel();

    void reset() override;
    void setOutputLevel(Gbs::WaveVolume level);
    void setWaveform(uint8_t buf[WAVE_RAMSIZE]);
    void step(unsigned cycles) override;

private:
    Gbs::WaveVolume outputLevel;
    uint8_t wavedata[WAVE_RAMSIZE];
    unsigned waveIndex;

};


class NoiseChannel : public EnvChannel {
    uint8_t scf;
    Gbs::NoiseSteps stepSelection;
    uint8_t drf;

    uint16_t lfsr;
    unsigned shiftCounter;
    unsigned shiftCounterMax;

public:
    
    NoiseChannel();

    void reset() override;
    void setDrf(uint8_t drf);
    void setNoise(uint8_t noiseReg);
    void setScf(uint8_t scf);
    void setStepSelection(Gbs::NoiseSteps steps);
    void step(unsigned cycles) override;
};


struct ChannelFile {
    SweepPulseChannel ch1;
    PulseChannel ch2;
    WaveChannel ch3;
    NoiseChannel ch4;
};


class Sequencer {
    unsigned freqCounter;
    unsigned stepCounter;
    ChannelFile &cf;

public:
    Sequencer(ChannelFile &cf);

    void reset();
    void step(unsigned cycles);
};


class Mixer {

public:

    Mixer();

    void getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight);
    void setEnable(Gbs::OutputFlags flags);
    void setEnable(ChType ch, Gbs::Terminal term, bool enabled);
    void setTerminalEnable(Gbs::Terminal term, bool enabled);
    void setTerminalVolume(Gbs::Terminal term, uint8_t volume);

private:
    bool s01enable, s02enable;
    uint8_t s01vol, s02vol;
    std::underlying_type<Gbs::OutputFlags>::type outputStat;

};


class Synth {
    ChannelFile cf;
    Mixer mixer;
    Sequencer sequencer;

    float samplingRate;
    unsigned stepsPerSample;

public:
    Synth(float samplingRate);

    ChannelFile& getChannels();
    Mixer& getMixer();
    Sequencer& getSequencer();

    void step(float &left, float &right);

    void fill(float leftBuf[], float rightBuf[], size_t nsamples);

    void fill(float buf[], size_t nsamples);
};


// utility functions

float fromGbFreq(uint16_t value);

uint16_t toGbFreq(float value);

}