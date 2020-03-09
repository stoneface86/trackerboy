
#pragma once

#include "trackerboy/gbs.hpp"

#include <cstdint>


namespace trackerboy {


class Envelope {

public:

    Envelope();

    void reset();

    void setRegister(uint8_t reg);

    void trigger();

    uint8_t value();

private:

    uint8_t mEnvelope;
    Gbs::EnvMode mEnvMode;
    uint8_t mEnvLength;

    uint8_t mEnvCounter;


};




}