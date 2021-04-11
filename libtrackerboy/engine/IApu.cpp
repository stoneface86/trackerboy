#include "trackerboy/engine/IApu.hpp"

namespace trackerboy {

IApu::IApu() {
}

IApu::~IApu() {

}


NullApu::NullApu() :
    IApu()
{
}

NullApu::~NullApu() {

}

uint8_t NullApu::readRegister(uint8_t reg) {
    return (uint8_t)0;
}

void NullApu::writeRegister(uint8_t reg, uint8_t value) {
    (void)reg;
    (void)value;
    // do nothing
}

GbApu::GbApu(gbapu::Apu &apu) :
    IApu(),
    mApu(apu)
{
}

GbApu::~GbApu() {

}

uint8_t GbApu::readRegister(uint8_t reg) {
    return mApu.readRegister(reg);
}

void GbApu::writeRegister(uint8_t reg, uint8_t value) {
    mApu.writeRegister(reg, value);
}

}
