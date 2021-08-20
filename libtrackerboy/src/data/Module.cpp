
#include "trackerboy/data/Module.hpp"

#include "internal/endian.hpp"
#include "internal/enumutils.hpp"
#include "internal/fileformat/fileformat.hpp"
#include "internal/fileformat/payload/payload.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>


namespace trackerboy {


Module::Module() noexcept :
    mSongs(),
    mInstrumentTable(),
    mWaveformTable(),
    mVersion(VERSION),
    mRevisionMajor(FILE_REVISION_MAJOR),
    mRevisionMinor(FILE_REVISION_MINOR),
    mTitle(),
    mArtist(),
    mCopyright(),
    mComments(),
    mSystem(System::dmg),
    mCustomFramerate(30)
{
}

void Module::clear() noexcept {
    mVersion = VERSION;
    mRevisionMajor = FILE_REVISION_MAJOR;
    mRevisionMinor = FILE_REVISION_MINOR;
    mTitle.clear();
    mArtist.clear();
    mCopyright.clear();
    mSongs.clear();
    mInstrumentTable.clear();
    mWaveformTable.clear();
    mSystem = System::dmg;
    mCustomFramerate = 30;
}

InfoStr const& Module::artist() const noexcept {
    return mArtist;
}

InfoStr const& Module::title() const noexcept {
    return mTitle;
}

InfoStr const& Module::copyright() const noexcept {
    return mCopyright;
}

std::string const& Module::comments() const noexcept {
    return mComments;
}

Version Module::version() const noexcept {
    return mVersion;
}

uint8_t Module::revisionMajor() const noexcept {
    return mRevisionMajor;
}

uint8_t Module::revisionMinor() const noexcept {
    return mRevisionMinor;
}

SongList& Module::songs() noexcept {
    return mSongs;
}

SongList const& Module::songs() const noexcept {
    return mSongs;
}

InstrumentTable& Module::instrumentTable() noexcept {
    return mInstrumentTable;
}

InstrumentTable const& Module::instrumentTable() const noexcept {
    return mInstrumentTable;
}

WaveformTable& Module::waveformTable() noexcept {
    return mWaveformTable;
}

WaveformTable const& Module::waveformTable() const noexcept {
    return mWaveformTable;
}

float Module::framerate() const noexcept {
    switch (mSystem) {
        case System::dmg:
            return GB_FRAMERATE_DMG;
        case System::sgb:
            return GB_FRAMERATE_SGB;
        default:
            return (float)mCustomFramerate;
    }
}

System Module::system() const noexcept {
    return mSystem;
}

int Module::customFramerate() const noexcept {
    return mCustomFramerate;
}

void Module::setArtist(InfoStr const& artist) noexcept {
    mArtist = artist;
}

void Module::setCopyright(InfoStr const& copyright) noexcept {
    mCopyright = copyright;
}

void Module::setTitle(InfoStr const& title) noexcept {
    mTitle = title;
}

void Module::setComments(std::string const& comments) noexcept {
    mComments = comments;
}

void Module::setComments(std::string &&comments) noexcept {
    mComments = std::move(comments);
}

void Module::setFramerate(System system) noexcept {
    mSystem = system;
}

void Module::setFramerate(int rate) {
    if (rate <= 0 || rate >= std::numeric_limits<uint16_t>::max()) {
        throw std::invalid_argument("invalid framerate given");
    }

    mSystem = System::custom;
    mCustomFramerate = rate;
}

FormatError Module::deserialize(std::istream &stream) noexcept {

    // read in the header
    Header header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!stream.good()) {
        return FormatError::readError;
    }

    // check the signature
    if (header.current.signature != FILE_SIGNATURE) {
        return FormatError::invalidSignature;
    }

    // check major revision
    auto const revMajor = header.current.revMajor;
    if (revMajor > FILE_REVISION_MAJOR) {
        // future revision, do not attempt to read file
        return FormatError::invalidRevision;
    } else if (!upgradeHeader(header)) {
        // upgrade failed
        return FormatError::cannotUpgrade;
    }

    mTitle = header.current.title;
    mArtist = header.current.artist;
    mCopyright = header.current.copyright;

    if (header.current.system <= +System::custom) {
        mSystem = static_cast<System>(header.current.system);
        if (mSystem == System::custom) {
            auto framerate = correctEndian(header.current.customFramerate);
            if (framerate == 0) {
                return FormatError::invalid;
            }
            mCustomFramerate = (int)framerate;
        }
    }

    if (header.current.icount > MAX_INSTRUMENTS || header.current.wcount > MAX_WAVEFORMS) {
        // too many instruments or waveforms
        return FormatError::invalid;
    }

    mInstrumentTable.clear();
    mWaveformTable.clear();

    int const songCount = unbias<int>(header.rev1.scount);
    // clear the song list, adding songCount new songs
    mSongs.clear(songCount);

    FormatError error;
    switch (revMajor) {
        case 0:
            error = deserializePayload0(*this, header, stream);
            break;
        default:
            // current major
            error = deserializePayload1(*this, header, stream);
            break;
    }

    if (error != FormatError::none) {
        return error;
    }

    if (revMajor == 0) {
        // major 0 had no file terminator, just check for EOF
        if (stream.peek() == EOF) {
            return FormatError::none;
        } else {
            // if we get here, we have successfully read the file, but
            // there is some unknown data remaining so treat this as an error
            return FormatError::invalid;
        }
    } else {
        // check for a file terminator, which is the signature reversed
        Signature terminator;
        stream.read(terminator.data(), terminator.size());
        if (!stream.good()) {
            return FormatError::invalid;
        }

        if (std::equal(terminator.begin(), terminator.end(), FILE_SIGNATURE.rbegin())) {
            return FormatError::none;
        } else {
            return FormatError::invalid; // terminator did not match!
        }

    }

}

FormatError Module::serialize(std::ostream &stream) const noexcept {

    // setup the header
    Header header;

    // signature
    header.current.signature = FILE_SIGNATURE;

    // version information (saving always overrides what was loaded)
    header.current.versionMajor = correctEndian(VERSION.major);
    header.current.versionMinor = correctEndian(VERSION.minor);
    header.current.versionPatch = correctEndian(VERSION.patch);

    // file information

    // revision gets set to the current revision
    // serializing always uses the current revision
    header.current.revMajor = FILE_REVISION_MAJOR;
    header.current.revMinor = FILE_REVISION_MINOR;

    header.current.title = mTitle;
    header.current.artist = mArtist;
    header.current.copyright = mCopyright;

    // set the table counts. scount is biased since we must have at least 1
    // song.
    header.current.icount = (uint8_t)mInstrumentTable.size();
    header.current.scount = bias(mSongs.size());
    header.current.wcount = (uint8_t)mWaveformTable.size();

    header.current.system = +mSystem;
    if (mSystem == System::custom) {
        header.current.customFramerate = correctEndian((uint16_t)mCustomFramerate);
    }

    // reserved was zero'd on initialization of header

    // write the header
    stream.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    if (!stream.good()) {
        return FormatError::writeError;
    }

    if (!serializePayload(*this, stream)) {
        return FormatError::writeError;
    }
    

    // add a terminator, the signature reversed
    Signature terminator;
    std::copy(FILE_SIGNATURE.rbegin(), FILE_SIGNATURE.rend(), terminator.begin());
    stream.write(terminator.data(), terminator.size());
    if (!stream.good()) {
        return FormatError::writeError;
    }
    
    return FormatError::none;
}

}
