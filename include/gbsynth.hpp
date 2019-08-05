
#pragma once

#include <cstdint>
#include <type_traits>
using std::uint8_t;
using std::uint16_t;

namespace gbsynth {

enum class Duty {
    p125 = 0,
    p25  = 1,
    p50  = 2,
    p75  = 3
};

enum class EnvMode {
    attenuate = 0,
    amplify = 1
};

enum class SweepMode {
    addition = 0,
    subtraction = 1
};

enum class StepCount {
    steps15 = 0,
    steps7 = 1
};

enum class WaveVolume {
    mute    = 0,
    full    = 1,
    half    = 2,
    quarter = 3
};

enum class Terminal {
    s01,
    s02,
    left  = s01,   // todo: verify that s01 = left
    right = s02
};

enum class OutputFlags : uint8_t {
    left1 = 0x1,
    left2 = 0x2,
    left3 = 0x4,
    left4 = 0x8,
    right1 = 0x10,
    right2 = 0x20,
    right3 = 0x40,
    right4 = 0x80,
    both1 = left1 | right1,
    both2 = left2 | right2,
    both3 = left3 | right3,
    both4 = left4 | right4,
    all_on = 0xFF,
    all_off = 0x0
    
};

constexpr inline OutputFlags operator |(OutputFlags lhs, OutputFlags rhs) {
    return static_cast<OutputFlags>(
        static_cast<std::underlying_type<OutputFlags>::type>(lhs) |
        static_cast<std::underlying_type<OutputFlags>::type>(rhs)
    );
}

constexpr inline OutputFlags operator &(OutputFlags lhs, OutputFlags rhs) {
    return static_cast<OutputFlags>(
        static_cast<std::underlying_type<OutputFlags>::type>(lhs) &
        static_cast<std::underlying_type<OutputFlags>::type>(rhs)
        );
}

constexpr inline OutputFlags operator ~(OutputFlags rhs) {
    return static_cast<OutputFlags>(
        ~static_cast<std::underlying_type<OutputFlags>::type>(rhs)
        );
}

constexpr inline OutputFlags& operator &=(OutputFlags &lhs, OutputFlags rhs) {
    lhs = static_cast<OutputFlags>(
            static_cast<std::underlying_type<OutputFlags>::type>(lhs) &
            static_cast<std::underlying_type<OutputFlags>::type>(rhs)
    );
    return lhs;
}

constexpr inline OutputFlags& operator |=(OutputFlags &lhs, OutputFlags rhs) {
    lhs = static_cast<OutputFlags>(
        static_cast<std::underlying_type<OutputFlags>::type>(lhs) |
        static_cast<std::underlying_type<OutputFlags>::type>(rhs)
        );
    return lhs;
}




enum Constants {
    // maximum values for parameters
    MAX_SWEEP_TIME      = 0x7,
    MAX_SWEEP_SHIFT     = 0x7,
    MAX_LENGTH          = 0x3F,
    MAX_ENV_STEPS       = 0xF,
    MAX_ENV_LENGTH      = 0x7,
    MAX_FREQUENCY       = 0x7FF,
    MAX_WAVE_LENGTH     = 0xFF,
    MAX_VOLUME          = 0x7,
    MAX_SCF             = 0xD,

    // defaults
    DEFAULT_FREQUENCY   = 0,
    DEFAULT_DUTY        = Duty::p75,
    DEFAULT_ENV_STEPS   = 0,
    DEFAULT_ENV_LENGTH  = 0,
    DEFAULT_ENV_MODE    = EnvMode::attenuate,
    DEFAULT_SWEEP_TIME  = MAX_SWEEP_TIME,
    DEFAULT_SWEEP_MODE  = SweepMode::addition,
    DEFAULT_SWEEP_SHIFT = 0,
    DEFAULT_WAVE_LEVEL  = WaveVolume::full,
    DEFAULT_TERM_ENABLE = false,
    DEFAULT_TERM_VOLUME = MAX_VOLUME,
    DEFAULT_SCF         = 0,
    DEFAULT_STEP_COUNT  = StepCount::steps15,
    DEFAULT_DRF         = 0,

    // sample values
    SAMPLE_GND          = 0x8,
    SAMPLE_MAX          = 0xF,
    SAMPLE_MIN          = 0x0,

    // # of entries in waveform ram (sound3)
    WAVE_SIZE           = 32,
    WAVE_RAMSIZE        = 16
};

class Channel {
    uint8_t lengthCounter;
    bool continuous;
    bool enabled;

protected:
    uint8_t currentSample;
    uint8_t length;

    Channel();

    virtual uint8_t generate(unsigned cycles) = 0;

public:
    virtual ~Channel() = default;

    void disable();
    uint8_t getCurrentSample();
    virtual float getCurrentVolume();
    void setLength(uint8_t length);
    void setContinuousOutput(bool continuous);
    void lengthStep();
    
    virtual void reset();
    void step(unsigned cycles);
};

class EnvChannel : public Channel {
    uint8_t envCounter;
        
public:
    virtual ~EnvChannel() = default;

    float getCurrentVolume() override;
    void setEnv(uint8_t envReg);
    void setEnvStep(uint8_t step);
    void setEnvMode(EnvMode mode);
    void setEnvLength(uint8_t length);
    void envStep();
    void reset() override;

protected:
    uint8_t envelope;
    EnvMode envMode;
    uint8_t envLength;

    EnvChannel();
};

#define calcFreqMax(f,m) ((2048 - f) * m)
template <unsigned multiplier>
class FreqChannel {
protected:
    uint16_t frequency;
    unsigned freqCounter;
    unsigned freqCounterMax;

public:
    FreqChannel() :
        frequency(DEFAULT_FREQUENCY),
        freqCounter(0),
        freqCounterMax(calcFreqMax(frequency, multiplier))
    {
    }

    uint16_t getFrequency() {
        return frequency;
    }

    void setFrequency(uint16_t _frequency) {
        frequency = _frequency;
        freqCounterMax = calcFreqMax(frequency, multiplier);
    }
};
#undef calcFreqMax

class PulseChannel : public EnvChannel, public FreqChannel<4> {
    Duty duty;
    unsigned dutyCounter;

public:
    PulseChannel();

    void setDuty(Duty duty);
    virtual void reset() override;

protected:
    uint8_t generate(unsigned cycles) override;
    
};

class SweepPulseChannel : public PulseChannel {
    SweepMode sweepMode;
    uint8_t sweepTime;
    uint8_t sweepShift;

    uint8_t sweepCounter;

public:
    SweepPulseChannel();

    void reset() override;
    void setSweep(uint8_t sweepReg);
    void setSweepTime(uint8_t ts);
    void setSweepMode(SweepMode mode);
    void setSweepShift(uint8_t n);
    void sweepStep();
};


class WaveChannel : public Channel, public FreqChannel<2> {
    WaveVolume outputLevel;
    uint8_t wavedata[WAVE_RAMSIZE];
    unsigned waveIndex;

public:
    WaveChannel();

    void setOutputLevel(WaveVolume level);
    void setWaveform(uint8_t buf[WAVE_RAMSIZE]);
    void reset() override;

protected:
    uint8_t generate(unsigned cycles) override;
};

class NoiseChannel : public EnvChannel {
    uint8_t scf;
    StepCount stepSelection;
    uint8_t drf;

    uint16_t lfsr;
    unsigned shiftCounter;
    unsigned shiftCounterMax;
        
public:
    NoiseChannel();

    void setNoise(uint8_t noiseReg);
    void setScf(uint8_t scf);
    void setStepSelection(StepCount count);
    void setDrf(uint8_t drf);

    void reset() override;

protected:
    uint8_t generate(unsigned cycles) override;

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

    void step(unsigned cycles);
    void reset();
};

class Mixer {
    bool s01enable, s02enable;
    uint8_t s01vol, s02vol;
    OutputFlags outputStat;

public:
    Mixer();

    void setTerminalEnable(Terminal term, bool enabled);
    void setTerminalVolume(Terminal term, uint8_t volume);
    void setEnable(OutputFlags flags);

    void getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight);
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

    void fill(float leftBuf[], float rightBuf[], size_t nsamples);
};


// utility functions

float fromGbFreq(uint16_t value);

uint16_t toGbFreq(float value);
}