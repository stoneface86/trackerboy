
#include "trackerboy/synth/Envelope.hpp"


namespace trackerboy {

Envelope::Envelope() :
    mEnvCounter(0),
    mEnvelope(Gbs::DEFAULT_ENV_STEPS),
    mEnvLength(Gbs::DEFAULT_ENV_LENGTH),
    mEnvMode(Gbs::DEFAULT_ENV_MODE)
{
}

void Envelope::reset() {
    mEnvCounter = 0;
}

void Envelope::setRegister(uint8_t envReg) {
    mEnvLength = (envReg & 0x7);
    mEnvMode = static_cast<Gbs::EnvMode>((envReg >> 3) & 1);
    mEnvelope = (envReg >> 4);
}

void Envelope::trigger() {
    if (mEnvLength) {
        // do nothing if envLength == 0
        if (++mEnvCounter == mEnvLength) {
            mEnvCounter = 0;
            if (mEnvMode == Gbs::ENV_AMPLIFY) {
                if (mEnvelope < Gbs::SAMPLE_MAX) {
                    ++mEnvelope;
                }
            } else {
                if (mEnvelope > Gbs::SAMPLE_MIN) {
                    --mEnvelope;
                }
            }
        }
    }
}

uint8_t Envelope::value() {
    return mEnvelope;
}



}