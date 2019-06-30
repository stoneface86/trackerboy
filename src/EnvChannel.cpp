
#include "gbsynth.h"

#include <cmath>

namespace gbsynth {

    EnvChannel::EnvChannel() : Channel() {
        envCounter = 0;
        envelope = 0;

        envLength = DEFAULT_ENV_LENGTH;
        envMode = (EnvMode)DEFAULT_ENV_MODE;
        envSteps = DEFAULT_ENV_STEPS;
    }

    uint8_t EnvChannel::apply(uint8_t sample) {
        // TODO
        return 0;
    }

    void EnvChannel::setEnvLength(uint8_t length) {
        if (length > MAX_ENV_LENGTH) {
            length = MAX_ENV_LENGTH;
        }
        envLength = length;
    }

    void EnvChannel::setEnvMode(EnvMode mode) {
        this->envMode = mode;
    }

    void EnvChannel::setEnvStep(uint8_t step) {
        if (step > MAX_ENV_STEPS) {
            step = MAX_ENV_STEPS;
        }
        envSteps = step;
        envCounter = step;
    }

    void EnvChannel::envStep() {
        // TODO
    }
    
    /*uint8_t EnvChannel::encodeEnvRegister() {
        uint8_t reg;
        reg = envSteps << 4;
        reg |= envMode << 3;
        reg |= envLength;
        return reg;
    }*/

    void EnvChannel::reset() {
        Channel::reset();
        envCounter = envSteps;
        
    }

}