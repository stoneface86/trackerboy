/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

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
