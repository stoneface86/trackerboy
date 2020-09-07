
#pragma once

#include "trackerboy/data/Table.hpp"
#include "trackerboy/data/Song.hpp"
#include "trackerboy/version.hpp"

#include <fstream>
#include <string>
#include <vector>


namespace trackerboy {


class Module {

public:
    Module() noexcept;
    ~Module() noexcept;

    void clear() noexcept;

    std::string artist() const noexcept;

    std::string title() const noexcept;

    std::string copyright() const noexcept;

    Version version() const noexcept;

    uint8_t revision() const noexcept;

    std::vector<Song>& songs() noexcept;

    WaveTable& waveTable() noexcept;

    InstrumentTable& instrumentTable() noexcept;

    FormatError deserialize(std::istream &stream) noexcept;

    FormatError serialize(std::ostream &stream) noexcept;

    void setArtist(std::string artist) noexcept;

    void setTitle(std::string title) noexcept;

    void setCopyright(std::string copyright) noexcept;


private:

    std::vector<Song> mSongs;
    WaveTable mWaveTable;
    InstrumentTable mInstrumentTable;

    // header settings
    Version mVersion;
    uint8_t mRevision;
    // information about the module (same format as *.gbs)
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;

};


}
