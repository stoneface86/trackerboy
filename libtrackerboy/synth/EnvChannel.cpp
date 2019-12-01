
#include "sampletable.hpp"

#include "trackerboy/synth/EnvChannel.hpp"
#include "trackerboy/gbs.hpp"

#include <cmath>

#define adjustSampleTable() SAMPLE_TABLE + (static_cast<size_t>(mEnvelope) * 16)

namespace trackerboy {

static const float ENV_TABLE[16] = {
    0.0f,
    1.0f / 15.0f,
    2.0f / 15.0f,
    3.0f / 15.0f,
    4.0f / 15.0f,
    5.0f / 15.0f,
    6.0f / 15.0f,
    7.0f / 15.0f,
    8.0f / 15.0f,
    9.0f / 15.0f,
    10.0f / 15.0f,
    11.0f / 15.0f,
    12.0f / 15.0f,
    13.0f / 15.0f,
    14.0f / 15.0f,
    1.0f
};


EnvChannel::EnvChannel() : 
    mEnvCounter(0),
    mEnvelope(Gbs::DEFAULT_ENV_STEPS),
    mEnvLength(Gbs::DEFAULT_ENV_LENGTH),
    mEnvMode(Gbs::DEFAULT_ENV_MODE),
    Channel() 
{
    mSampleTable = SAMPLE_TABLE; // start at envelope 0
}

void EnvChannel::envStep() {
    if (mEnvLength) {
        // do nothing if envLength == 0
        if (mEnvCounter == mEnvLength) {
            mEnvCounter = 0;
            if (mEnvMode == Gbs::ENV_AMPLIFY) {
                if (mEnvelope < Gbs::SAMPLE_MAX) {
                    ++mEnvelope;
                    mSampleTable += 16;
                }
            } else {
                if (mEnvelope > Gbs::SAMPLE_MIN) {
                    --mEnvelope;
                    mSampleTable -= 16;
                }
            }
        } else {
            ++mEnvCounter;
        }
    }
}

void EnvChannel::reset() {
    Channel::reset();
    mEnvCounter = 0;
}

void EnvChannel::setEnv(uint8_t envReg) {
    mEnvLength = (envReg & 0x7);
    mEnvMode = static_cast<Gbs::EnvMode>((envReg >> 3) & 1);
    mEnvelope = (envReg >> 4);
    mSampleTable = adjustSampleTable();
}

void EnvChannel::setEnvLength(uint8_t _envLength) {
    if (_envLength > Gbs::MAX_ENV_LENGTH) {
        _envLength = Gbs::MAX_ENV_LENGTH;
    }
    mEnvLength = _envLength;
}

void EnvChannel::setEnvMode(Gbs::EnvMode mode) {
    this->mEnvMode = mode;
}

void EnvChannel::setEnvStep(uint8_t step) {
    if (step > Gbs::MAX_ENV_STEPS) {
        step = Gbs::MAX_ENV_STEPS;
    }
    mEnvelope = step;
    mSampleTable = adjustSampleTable();
}

}