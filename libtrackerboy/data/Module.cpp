
#include "trackerboy/data/Module.hpp"
#include "trackerboy/fileformat.hpp"

#include "internal/endian.hpp"
#include "internal/enumutils.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <variant>


namespace trackerboy {


Module::Module() noexcept :
    mSong(),
    mInstrumentTable(),
    mWaveformTable(),
    mVersion(VERSION),
    mRevision(FILE_REVISION),
    mTitle(),
    mArtist(),
    mCopyright(),
    mComments(),
    mSystem(System::dmg),
    mCustomFramerate(30)
{
}

Module::~Module() noexcept {
}

void Module::clear() noexcept {
    mVersion = VERSION;
    mRevision = FILE_REVISION;
    mTitle = "";
    mArtist = "";
    mCopyright = "";
    mSong.reset();
    mInstrumentTable.clear();
    mWaveformTable.clear();
    mSystem = System::dmg;
    mCustomFramerate = 30;
}

std::string const& Module::artist() const noexcept {
    return mArtist;
}

std::string const& Module::title() const noexcept {
    return mTitle;
}

std::string const& Module::copyright() const noexcept {
    return mCopyright;
}

std::string const& Module::comments() const noexcept {
    return mComments;
}

Version Module::version() const noexcept {
    return mVersion;
}

uint8_t Module::revision() const noexcept {
    return mRevision;
}

Song& Module::song() noexcept {
    return mSong;
}

Song const& Module::song() const noexcept {
    return mSong;
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

void Module::setArtist(std::string const& artist) noexcept {
    mArtist = artist;
}

void Module::setCopyright(std::string const& copyright) noexcept {
    mCopyright = copyright;
}

void Module::setTitle(std::string const& title) noexcept {
    mTitle = title;
}

void Module::setComments(std::string const& comments) noexcept {
    mComments = comments;
}

void Module::setFramerate(System system) noexcept {
    mSystem = system;
}

void Module::setFramerate(int rate) {
    if (rate <= 0 || rate >= UINT16_MAX) {
        throw std::invalid_argument("invalid framerate given");
    }

    mSystem = System::custom;
    mCustomFramerate = rate;
}

// ---- Serialization ----


class IOError : public std::exception {

};

class BoundsError : public std::exception {

};


//
// Class for reading data from a "block"
// attempting to read past the block results in a BoundsError being thrown
//
class InputBlock {

public:

    
    InputBlock(std::istream &stream) :
        mStream(stream),
        mSize(0),
        mPosition(0)
    {
        
    }

    size_t size() const {
        return mSize;
    }

    BlockId begin() {
        // read the block type
        BlockId id;
        mStream.read(reinterpret_cast<char*>(&id), sizeof(id));
        if (!mStream.good()) {
            throw IOError();
        }

        BlockSize size;
        mStream.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (!mStream.good()) {
            throw IOError();
        }
        mSize = correctEndian(size);
        mPosition = 0;

        return correctEndian(id);
    }

    template <typename T>
    void read(T& ref) {
        read(sizeof(T), reinterpret_cast<char*>(&ref));
    }

    template <typename T>
    void read(size_t count, T *data) {
        read(count * sizeof(T), reinterpret_cast<char*>(data));
    }

    

    //
    // Returns true if the entire block has been read
    //
    bool finished() {
        return mPosition == mSize;
    }


private:
    std::istream &mStream;
    size_t mSize;
    size_t mPosition;

};

template <>
void InputBlock::read(size_t count, char *data) {
    auto newpos = mPosition + count;
    if (newpos > mSize) {
        // attempted to read past the block, error!
        // the data is corrupted or ill-formed
        throw BoundsError();
    }

    mStream.read(data, count);
    if (!mStream.good()) {
        throw IOError();
    }
    mPosition = newpos;
}

//
// Class for writing data and encapsulating it in a "block"
// A block contains a 4-byte id and a 4-byte size.
//
class OutputBlock {

public:

    OutputBlock(std::ostream &stream) :
        mStream(stream),
        mLengthPos(0),
        mSize(0)
    {
        
    }

    void begin(BlockId id) {
        BlockId idOut = correctEndian(id);
        mStream.write(reinterpret_cast<const char*>(&idOut), sizeof(idOut));
        if (!mStream.good()) {
            throw IOError();
        }

        mLengthPos = mStream.tellp();

        uint32_t size = 0;
        mStream.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (!mStream.good()) {
            throw IOError();
        }

        mSize = 0;
    }

    //
    // Finish writing the block. The block's length field is updated with
    // this block's length.
    //
    void finish() {
        if (mSize) {
            auto oldpos = mStream.tellp();
            mStream.seekp(mLengthPos);
            BlockSize size = correctEndian((uint32_t)mSize);
            mStream.write(reinterpret_cast<const char*>(&size), sizeof(size));
            if (!mStream.good()) {
                throw IOError();
            }

            mStream.seekp(oldpos);
        }
    }

    template <typename T>
    void write(T const& ref) {
        write(sizeof(T), reinterpret_cast<const char*>(&ref));
    }

    template <typename T>
    void write(size_t count, T const *data) {
        write(sizeof(T) * count, reinterpret_cast<const char*>(data));
    }

    


private:
    std::ostream &mStream;
    std::streampos mLengthPos;
    size_t mSize;
};

template <>
void OutputBlock::write(size_t count, const char* data) {
    mStream.write(data, count);
    if (!mStream.good()) {
        throw IOError();
    }
    mSize += count;
}

#pragma pack(push, 1)

struct SongFormat {
    uint8_t rowsPerBeat;
    uint8_t rowsPerMeasure;
    uint8_t speed;
    uint8_t patternCount;
    uint8_t rowsPerTrack;
    uint16_t numberOfTracks;
    // order data...Order[patternCount]
    // pattern data TrackFormat[numberOfTracks]
};

struct TrackFormat {
    uint8_t channel;
    uint8_t trackId;
    uint8_t rows;
    // row data: TrackRow[rows]
};

struct RowFormat {
    uint8_t rowno;
    TrackRow rowdata;
};

struct InstrumentFormat {
    uint8_t channel;
    bool envelopeEnabled;
    uint8_t envelope;
    // sequence data follows
};

struct SequenceFormat {
    uint16_t length;
    bool loopEnabled;
    uint8_t loopIndex;
    // sequence data follows uint8_t[length]
};

#pragma pack(pop)

void serializeString(OutputBlock &block, std::string const& str) {
    auto size = str.size();
    if (size > 255) {
        size = 255;
    }

    uint8_t sizeOut = (uint8_t)size;
    block.write(sizeOut);
    if (size) {
        block.write(size, str.c_str());
    }
}

std::string deserializeString(InputBlock &block) {
    uint8_t size;
    block.read(size);

    std::string str;
    str.resize(size);
    block.read(size, str.data());
    
    return str;
}

template <class T>
void writeListIndex(OutputBlock &block, T const& list) {
    for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
        auto item = list[id];
        if (item) {
            block.write(id);
            serializeString(block, list[id]->name());
        }
    }
}

template <class T>
void readListIndex(InputBlock &block, size_t count, T &list) {
    while (count--) {
        uint8_t id;
        block.read(id);
        auto &item = list.insert(id);
        item.setName(deserializeString(block));
    }


}

// most counter fields in the file format range from 1-256, but use a single byte for encoding
// the counter is biased by subtracting 1 such that 1...256 is represented by 0...255

// when writing: bias
// when reading: unbias

// returns the unbiased form of the given biased number
template <typename T>
constexpr inline T unbias(uint8_t num) {
    return (T)num + 1;
}

// return the biased form of the given number
// assumptions: num is an integral type whose value is between 1 and 256, inclusive.
template <typename T>
constexpr inline uint8_t bias(T num) {
    return (uint8_t)(num - 1);
}


FormatError Module::deserialize(std::istream &stream) noexcept {

    // read in the header
    Header header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!stream.good()) {
        return FormatError::readError;
    }

    // check the signature
    if (!std::equal(header.signature, header.signature + Header::SIGNATURE_SIZE, FILE_SIGNATURE)) {
        return FormatError::invalidSignature;
    }

    // check revision, do not attempt to parse future versions
    if (header.revision > FILE_REVISION) {
        return FormatError::invalidRevision;
    }


    mRevision = header.revision;

    // ensure strings are null terminated
    header.title[Header::TITLE_LENGTH - 1] = '\0';
    header.artist[Header::ARTIST_LENGTH - 1] = '\0';
    header.copyright[Header::COPYRIGHT_LENGTH - 1] = '\0';

    mTitle = std::string(header.title);
    mArtist = std::string(header.artist);
    mCopyright = std::string(header.copyright);

    if (header.system <= +System::custom) {
        mSystem = static_cast<System>(header.system);
        if (mSystem == System::custom) {
            auto framerate = correctEndian(header.customFramerate);
            if (framerate == 0) {
                return FormatError::invalid;
            }
            mCustomFramerate = (int)framerate;
        }
    }

    mSong.reset();
    mInstrumentTable.clear();
    mWaveformTable.clear();

    InputBlock block(stream);

    try {

        // "INDX"
        if (block.begin() != BLOCK_ID_INDEX) {
            return FormatError::invalid;
        }

        mSong.setName(deserializeString(block));
        
        readListIndex(block, header.numberOfInstruments, mInstrumentTable);
        readListIndex(block, header.numberOfWaveforms, mWaveformTable);

        if (!block.finished()) {
            return FormatError::invalid;
        }
            

        // "COMM"
        if (block.begin() != BLOCK_ID_COMMENT) {
            return FormatError::invalid;
        }

        // the entire block is the comments string
        auto commentsLength = block.size();
        mComments.resize(commentsLength);
        block.read(commentsLength, mComments.data());

        if (!block.finished()) {
            return FormatError::invalid;
        }
            

        // "SONG"
        if (block.begin() != BLOCK_ID_SONG) {
            return FormatError::invalid;
        }

        {

            // read in song settings
            SongFormat songFormat;
            block.read(songFormat);
            mSong.setRowsPerBeat(songFormat.rowsPerBeat);
            mSong.setRowsPerMeasure(songFormat.rowsPerMeasure);
            mSong.setSpeed(songFormat.speed);

            auto &pm = mSong.patterns();
            pm.setRowSize(unbias<uint16_t>(songFormat.rowsPerTrack));

            // read in the order
            {
                std::vector<OrderRow> orderData;
                orderData.resize(unbias<size_t>(songFormat.patternCount));
                for (auto &row : orderData) {
                    block.read(row);
                }
                // the order takes ownership of orderData
                mSong.order().setData(std::move(orderData));
            }


            // read in track data
            size_t const tracks = (size_t)correctEndian(songFormat.numberOfTracks);
            for (size_t i = 0; i != tracks; ++i) {
                TrackFormat trackFormat;
                block.read(trackFormat);
                if (trackFormat.channel > static_cast<uint8_t>(ChType::ch4)) {
                    return FormatError::unknownChannel;
                }
                auto &track = pm.getTrack(static_cast<ChType>(trackFormat.channel), trackFormat.trackId);
                for (size_t r = (size_t)trackFormat.rows + 1; r--; ) {
                    RowFormat rowFormat;
                    block.read(rowFormat);
                    track.replace(rowFormat.rowno, rowFormat.rowdata);
                }
            }

        }

        if (!block.finished()) {
            return FormatError::invalid;
        }
            

        // "INST"
        if (block.begin() != BLOCK_ID_INSTRUMENT) {
            return FormatError::invalid;
        }

        //for (auto id : mInstrumentList) {
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto inst = mInstrumentTable[id];
            if (inst) {
                InstrumentFormat format;
                block.read(format);
                if (format.channel > +ChType::ch4) {
                    return FormatError::unknownChannel;
                }
                inst->setChannel(static_cast<ChType>(format.channel));
                inst->setEnvelopeEnable(format.envelopeEnabled);
                inst->setEnvelope(format.envelope);
                
                for (size_t i = 0; i < Instrument::SEQUENCE_COUNT; ++i) {
                    auto &sequence = inst->sequence(i);
                    
                    SequenceFormat sequenceFmt;
                    block.read(sequenceFmt);
                    sequenceFmt.length = correctEndian(sequenceFmt.length);
                    if (sequenceFmt.length > Sequence::MAX_SIZE) {
                        return FormatError::invalid;
                    }

                    sequence.resize(sequenceFmt.length);
                    if (sequenceFmt.loopEnabled) {
                        sequence.setLoop(sequenceFmt.loopIndex);
                    }
                    auto &seqdata = sequence.data();
                    block.read(sequenceFmt.length, seqdata.data());
                }
            }
        }


        if (!block.finished()) {
            return FormatError::invalid;
        }
            

        // "WAVE"
        if (block.begin() != BLOCK_ID_WAVE) {
            return FormatError::invalid;
        }

        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto wave = mWaveformTable[id];
            if (wave) {
                block.read(wave->data());
            }
        }

        if (!block.finished()) {
            return FormatError::invalid;
        }
            

    } catch (IOError const& err) {
        (void)err;
        return FormatError::readError;
    } catch (BoundsError const& err) {
        (void)err;
        return FormatError::invalid;
    }
    
    if (stream.peek() == EOF) {
        return FormatError::none;
    } else {
        // if we get here, we have successfully read the file, but
        // there is some unknown data remaining so treat this as an error
        return FormatError::invalid;
    }
}

FormatError Module::serialize(std::ostream &stream) const noexcept {
    

    // setup the header

    Header header{ 0 };

    // signature
    std::copy(FILE_SIGNATURE, FILE_SIGNATURE + Header::SIGNATURE_SIZE, header.signature);

    // version information (saving always overrides what was loaded)
    header.versionMajor = correctEndian(mVersion.major);
    header.versionMinor = correctEndian(mVersion.minor);
    header.versionPatch = correctEndian(mVersion.patch);

    // file information

    // revision remains the same as the one that was loaded.
    // for new files, it is set to the current revision.
    header.revision = mRevision;

    #define copyStringToFixed(dest, string, count) do { \
            size_t len = std::min(count - 1, string.length()); \
            string.copy(dest, len); \
            dest[len] = '\0'; \
        } while (false)

    copyStringToFixed(header.title, mTitle, Header::TITLE_LENGTH);
    copyStringToFixed(header.artist, mArtist, Header::ARTIST_LENGTH);
    copyStringToFixed(header.copyright, mCopyright, Header::COPYRIGHT_LENGTH);

    header.numberOfInstruments = correctEndian((uint16_t)mInstrumentTable.size());
    header.numberOfWaveforms = correctEndian((uint16_t)mWaveformTable.size());

    header.system = +mSystem;
    if (mSystem == System::custom) {
        header.customFramerate = correctEndian((uint16_t)mCustomFramerate);
    }

    #undef copyStringToFixed

    // reserved was zero'd on initialization of header

    // write the header
    stream.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    if (!stream.good()) {
        return FormatError::writeError;
    }
    
    OutputBlock block(stream);
    try {

        // "INDX"
        block.begin(BLOCK_ID_INDEX);
        serializeString(block, mSong.name());
        
        writeListIndex(block, mInstrumentTable);
        writeListIndex(block, mWaveformTable);

        block.finish();
        

        // "COMM"
        block.begin(BLOCK_ID_COMMENT);
        block.write(mComments.size(), mComments.c_str());
        block.finish();
        

        // "SONG"
        block.begin(BLOCK_ID_SONG);
        {
            auto &order = mSong.order();
            auto &pm = mSong.patterns();

            SongFormat songHeader;
            songHeader.rowsPerBeat = (uint8_t)mSong.rowsPerBeat();
            songHeader.rowsPerMeasure = (uint8_t)mSong.rowsPerMeasure();
            songHeader.speed = mSong.speed();
            songHeader.patternCount = bias(order.size());
            songHeader.rowsPerTrack = bias(pm.rowSize());
            songHeader.numberOfTracks = correctEndian((uint16_t)pm.tracks());
            block.write(songHeader);

            // write out song order
            for (auto &orderRow : order.data()) {
                block.write(orderRow);
            }

            // write out all tracks
            // iterate all channels
            for (uint8_t ch = 0; ch <= +ChType::ch4; ++ch) {
                // track iterators for the current channel
                auto begin = pm.tracksBegin(static_cast<ChType>(ch));
                auto end = pm.tracksEnd(static_cast<ChType>(ch));

                // iterate all tracks for this channel
                for (auto pair = begin; pair != end; ++pair) {
                    // make sure track is non-empty, we only save non-empty tracks
                    if (pair->second.rowCount() > 0) {

                        TrackFormat trackFormat;
                        trackFormat.channel = ch;
                        trackFormat.trackId = pair->first;
                        trackFormat.rows = bias(pair->second.rowCount());
                        block.write(trackFormat);

                        // iterate all rows in this track
                        uint8_t rowno = 0;
                        for (auto &row : pair->second) {
                            if (!row.isEmpty()) {
                                RowFormat rowFormat;
                                rowFormat.rowno = rowno;
                                rowFormat.rowdata = row;
                                block.write(rowFormat);
                            }

                            ++rowno;
                        }
                    }
                }
            }

        }
        block.finish();
        

        // "INST"
        block.begin(BLOCK_ID_INSTRUMENT);
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto instrument = mInstrumentTable[id];
            if (instrument) {
                InstrumentFormat format;
                format.channel = +instrument->channel();
                format.envelopeEnabled = instrument->hasEnvelope();
                format.envelope = instrument->envelope();
                block.write(format);

                for (auto &sequence : instrument->sequences()) {
                    SequenceFormat sequenceFmt;
                    auto &seqdata = sequence.data();
                    sequenceFmt.length = correctEndian((uint16_t)seqdata.size());
                    auto loop = sequence.loop();
                    sequenceFmt.loopEnabled = loop.has_value();
                    sequenceFmt.loopIndex = loop.value_or((uint8_t)0);
                    block.write(sequenceFmt);
                    block.write(seqdata.size(), seqdata.data());
                }
            }
        }
        block.finish();
        

        // "WAVE"
        block.begin(BLOCK_ID_WAVE);
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto waveform = mWaveformTable[id];
            if (waveform) {
                block.write(waveform->data());
            }
        }
        block.finish();
        

    } catch (IOError const& err) {
        (void)err;
        return FormatError::writeError;
    }

    return FormatError::none;
}

}
