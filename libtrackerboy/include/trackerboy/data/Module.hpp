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

#include "trackerboy/data/InfoStr.hpp"
#include "trackerboy/data/Table.hpp"
#include "trackerboy/data/SongList.hpp"
#include "trackerboy/version.hpp"

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>

namespace trackerboy {


class Module {

public:

    // rule-of-zero

    Module() noexcept;

    void clear() noexcept;

    // header information

    InfoStr const& artist() const noexcept;

    InfoStr const& title() const noexcept;

    InfoStr const& copyright() const noexcept;

    std::string const& comments() const noexcept;

    Version version() const noexcept;

    int revisionMajor() const noexcept;

    int revisionMinor() const noexcept;

    float framerate() const noexcept;

    System system() const noexcept;

    int customFramerate() const noexcept;

    SongList& songs() noexcept;
    SongList const& songs() const noexcept;

    WaveformTable& waveformTable() noexcept;
    WaveformTable const& waveformTable() const noexcept;

    InstrumentTable& instrumentTable() noexcept;
    InstrumentTable const& instrumentTable() const noexcept;

    // File I/O

    FormatError deserialize(std::istream &stream) noexcept;

    FormatError serialize(std::ostream &stream) const noexcept;

    void setArtist(InfoStr const& artist) noexcept;

    void setTitle(InfoStr const& title) noexcept;

    void setCopyright(InfoStr const& copyright) noexcept;

    void setComments(std::string const& comments) noexcept;
    void setComments(std::string&& comments) noexcept;

    void setFramerate(System system) noexcept;

    void setFramerate(int rate);

private:

    SongList mSongs;

    InstrumentTable mInstrumentTable;
    WaveformTable mWaveformTable;

    // header settings
    Version mVersion;
    int mRevisionMajor;
    int mRevisionMinor;
    // information about the module (same format as *.gbs)
    InfoStr mTitle;
    InfoStr mArtist;
    InfoStr mCopyright;

    // user comments/info about the module itself
    std::string mComments;

    System mSystem;
    int mCustomFramerate;
};


}
