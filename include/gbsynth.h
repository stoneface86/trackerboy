
#pragma once

#include <cstdint>
#include <vector>
using std::uint8_t;
using std::uint16_t;
using std::vector;

namespace gbsynth {

    enum Duty {
        DUTY_125 = 0,
        DUTY_25  = 1,
        DUTY_50  = 2,
        DUTY_75  = 3
    };

    enum EnvMode {
        ENV_ATTENUATE = 0,
        ENV_AMPLIFY = 1
    };

    enum SweepMode {
        SWEEP_ADDITION = 0,
        SWEEP_SUBTRACTION = 1
    };

    enum StepCount {
        STEPS_15 = 0,
        STEPS_7 = 1
    };

    enum WaveformLevel {
        WAVE_MUTE    = 0,
        WAVE_WHOLE   = 1,
        WAVE_HALF    = 2,
        WAVE_QUARTER = 3
    };

    enum Constants {
        // maximum values for parameters
        MAX_SWEEP_TIME      = 0x7,
        MAX_SWEEP_SHIFT     = 0x7,
        MAX_LENGTH          = 0x3F,
        MAX_ENV_STEPS       = 0xF,
        MAX_ENV_LENGTH      = 0x7,
        MAX_FREQUENCY       = 0x7FF,
        MAX_WAVE_LENGTH     = 0xFF,

        // defaults
        DEFAULT_FREQUENCY   = 0,
        DEFAULT_DUTY        = DUTY_75,
        DEFAULT_ENV_STEPS   = 0,
        DEFAULT_ENV_LENGTH  = 0,
        DEFAULT_ENV_MODE    = ENV_ATTENUATE,
        DEFAULT_SWEEP_TIME  = MAX_SWEEP_TIME,
        DEFAULT_SWEEP_MODE  = SWEEP_ADDITION,
        DEFAULT_SWEEP_SHIFT = 0,

        // sample values
        SAMPLE_GND          = 0x8,
        SAMPLE_MAX          = 0xF,
        SAMPLE_MIN          = 0x0
    };

    struct Ch1Reg {
        uint8_t nr10;
        uint8_t nr11;
        uint8_t nr12;
        uint8_t nr13;
        uint8_t nr14;
    };

    struct Ch2Reg {
        uint8_t nr21;
        uint8_t nr22;
        uint8_t nr23;
        uint8_t nr24;
    };

    struct Ch3Reg {
        uint8_t nr30;
        uint8_t nr31;
        uint8_t nr32;
        uint8_t nr33;
        uint8_t nr34;
    };

    struct Ch4Reg {
        uint8_t nr41;
        uint8_t nr42;
        uint8_t nr43;
        uint8_t nr44;
    };

    union ChRegUnion {
        Ch1Reg ch1;
        Ch2Reg ch2;
        Ch3Reg ch3;
        Ch4Reg ch4;
    };

    class Oscillator {
        vector<uint8_t> referencePeriod;
        vector<uint8_t> period;
        size_t counter;
        float samplesPerPeriod;

        void setPeriod();

    public:
        Oscillator(uint8_t waveform[], size_t nsamples);

        void setFrequency(float samplingRate, float frequency);
        void setWaveform(uint8_t waveform[], size_t nsamples);
        uint8_t* fill(uint8_t buf[], size_t nsamples);
        void reset();
    };

    class Channel {
        unsigned samplesToOutput;
        unsigned sampleCounter;
    
    public:
        virtual void getRegisters(ChRegUnion &reg) = 0;
        void fill(uint8_t buf[], size_t bufsize);
        void setLength(uint8_t length);
        void setContinuousOutput(bool continuous);
        virtual void reset() = 0;

    protected:
        float samplingRate;
        uint8_t length;
        bool continuous;
        bool enabled;

        Channel(float samplingRate);

        virtual size_t generate(uint8_t buf[], size_t bufsize) = 0;  
    };

    class EnvChannel : public Channel {
        unsigned samplesPerStep;
        unsigned stepCounter;
        unsigned envCounter;
        float envelope;

    public:
        void setEnvStep(uint8_t step);
        void setEnvMode(EnvMode mode);
        void setEnvLength(uint8_t length);
        virtual void reset() override;
    
    protected:
        uint8_t envSteps;
        EnvMode envMode;
        uint8_t envLength;

        EnvChannel(float samplingRate);

        uint8_t encodeEnvRegister();
        void apply(uint8_t buf[], size_t bufsize);
    };

    class PulseChannel : public EnvChannel {
        uint16_t frequency;
        Duty duty;
        SweepMode sweepMode;
        uint8_t sweepTime;
        uint8_t sweepShift;

        Oscillator osc;

        unsigned samplesPerSweep;
        unsigned sweepCounter;
        bool sweepEnabled;

    protected:
        size_t generate(uint8_t buf[], size_t nsamples) override;

    public:
        PulseChannel(float samplingRate, bool enableSweep);

        void getRegisters(ChRegUnion &reg) override;
        void setDuty(Duty duty);
        void setFrequency(uint16_t frequency);
        void setFrequency(float frequency);
        void setSweepTime(uint8_t ts);
        void setSweepMode(SweepMode mode);
        void setSweepShift(uint8_t n);

        void reset() override;

    };

    class WaveChannel : Channel {

    };

    class NoiseChannel : EnvChannel {

    };


    // utility functions

    float fromGbFreq(uint16_t value);

    uint16_t toGbFreq(float value);

    void bufToFloat(uint8_t input[], float output[], size_t nsamples);

}