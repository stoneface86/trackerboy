
#include "trackerboy/synth/Envelope.hpp"


namespace trackerboy {

Envelope::Envelope() :
    mEnvelope(Gbs::DEFAULT_ENV_STEPS),
    mEnvMode(Gbs::DEFAULT_ENV_MODE),
    mEnvLength(Gbs::DEFAULT_ENV_LENGTH),
    mEnvCounter(0),
    mRegister(Gbs::DEFAULT_ENV_REGISTER)
{
}

uint8_t Envelope::readRegister() {
    return mRegister;
}

void Envelope::reset() {
    mRegister = Gbs::DEFAULT_ENV_REGISTER;
    restart();
}

void Envelope::restart() {
    mEnvCounter = 0;
    mEnvLength = (mRegister & 0x7);
    mEnvMode = static_cast<Gbs::EnvMode>((mRegister >> 3) & 1);
    mEnvelope = (mRegister >> 4);
}

void Envelope::writeRegister(uint8_t envReg) {
    mRegister = envReg;
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
