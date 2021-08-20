
#pragma once

#include "gbapu.hpp"

#include <cstdint>

namespace trackerboy {


//
// Apu interface
//
class IApu {

public:
    IApu();
    virtual ~IApu();

    virtual uint8_t readRegister(uint8_t reg) = 0;

    virtual void writeRegister(uint8_t reg, uint8_t value) = 0;

};

//
// Null Apu, all writes to this apu do nothing
//
class NullApu final : public IApu {

public:
    NullApu();
    ~NullApu();

    virtual uint8_t readRegister(uint8_t reg) override;

    virtual void writeRegister(uint8_t reg, uint8_t value) override;

};

//
// Wrapper Apu for gbapu::Apu
//
class GbApu final : public IApu {

public:
    GbApu(gbapu::Apu &apu);
    ~GbApu();

    virtual uint8_t readRegister(uint8_t reg) override;

    virtual void writeRegister(uint8_t reg, uint8_t value) override;

private:
    gbapu::Apu &mApu;
};


}
