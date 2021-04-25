/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
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

#include "trackerboy/data/Table.hpp"
#include "trackerboy/data/Song.hpp"
#include "trackerboy/version.hpp"

#include <cstddef>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace trackerboy {


class Module {

public:

    Module() noexcept;
    ~Module() noexcept;

    void clear() noexcept;

    // header information

    std::string const& artist() const noexcept;

    std::string const& title() const noexcept;

    std::string const& copyright() const noexcept;

    std::string const& comments() const noexcept;

    Version version() const noexcept;

    uint8_t revision() const noexcept;

    Song& song() noexcept;
    Song const& song() const noexcept;

    WaveformTable& waveformTable() noexcept;
    WaveformTable const& waveformTable() const noexcept;

    InstrumentTable& instrumentTable() noexcept;
    InstrumentTable const& instrumentTable() const noexcept;

    // File I/O

    FormatError deserialize(std::istream &stream) noexcept;

    FormatError serialize(std::ostream &stream) const noexcept;

    void setArtist(std::string const& artist) noexcept;

    void setTitle(std::string const& title) noexcept;

    void setCopyright(std::string const& copyright) noexcept;

    void setComments(std::string const& comments) noexcept;


private:

    Song mSong;

    InstrumentTable mInstrumentTable;
    WaveformTable mWaveformTable;

    // header settings
    Version mVersion;
    uint8_t mRevision;
    // information about the module (same format as *.gbs)
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;

    // user comments/info about the module itself
    std::string mComments;
};


}
