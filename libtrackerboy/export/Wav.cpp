
#include "trackerboy/export/Wav.hpp"
#include "trackerboy/fileformat.hpp"

#include <cassert>


#define correctEndianAndSet(lvar) lvar = correctEndian(lvar)

namespace {

#pragma pack(push, 1)

//
// Header for wav files with 32-bit floating point samples
//
struct WavHeader {

    // [B] indicates that this field is set in begin()
    // [F] indicates that this field is set in finish()
    // = indicates the field's initial value

    // RIFF chunk
    char riffId[4];             // "RIFF"
    uint32_t chunkSize;         // [F]
    char waveId[4];             // "WAVE"
    // fmt subchunk
    char fmtId[4];              // "fmt "
    uint32_t fmtChunkSize;      // = 18
    uint16_t fmtTag;            // = 0x3 for IEEE_FLOAT
    uint16_t fmtChannels;       // [B]
    uint32_t fmtSampleRate;     // [B]
    uint32_t fmtAvgBytesPerSec; // [B] = 4 * fmtSampleRate * fmtChannels
    uint16_t fmtBlockAlign;     // [B] = 4 * fmtChannels
    uint16_t fmtBitsPerSample;  // = 32
    uint16_t fmtCbSize;         // = 0
    // fact subchunk
    char factId[4];             // "fact"
    uint32_t factChunkSize;     // = 4
    uint32_t factSampleCount;   // [F]
    // data subchunk
    char dataId[4];             // "data"
    uint32_t dataChunkSize;     // [F]
    // sampled data follows...
    // extra padding byte if dataChunkSize is odd [F]

    WavHeader() :
        riffId{'R', 'I', 'F', 'F'},
        chunkSize(0),
        waveId{'W', 'A', 'V', 'E'},
        fmtId{'f', 'm', 't', ' '},
        fmtChunkSize(18),
        fmtTag(0x3),
        fmtChannels(0),
        fmtSampleRate(0),
        fmtAvgBytesPerSec(0),
        fmtBlockAlign(0),
        fmtBitsPerSample(32),
        fmtCbSize(0),
        factId{'f', 'a', 'c', 't'},
        factChunkSize(4),
        factSampleCount(0),
        dataId{'d', 'a', 't', 'a'},
        dataChunkSize(0)
    {
    }

};

#pragma pack(pop)

}


namespace trackerboy {


Wav::Wav(std::ostream &stream, uint16_t channels, uint32_t samplingRate) :
    mStream(stream),
    mSampleCount(0),
    mChannels(channels),
    mSamplingRate(samplingRate)
{


}

void Wav::begin() {

    WavHeader header;
    header.fmtChannels = mChannels;
    header.fmtSampleRate = mSamplingRate;
    uint32_t bytesPerChannel = mChannels * 4;
    header.fmtAvgBytesPerSec = bytesPerChannel * mSamplingRate;
    header.fmtBlockAlign = bytesPerChannel;

    // convert everything to little endian if needed
    
    correctEndianAndSet(header.fmtChunkSize);
    correctEndianAndSet(header.fmtTag);
    correctEndianAndSet(header.fmtChannels);
    correctEndianAndSet(header.fmtSampleRate);
    correctEndianAndSet(header.fmtAvgBytesPerSec);
    correctEndianAndSet(header.fmtBlockAlign);
    correctEndianAndSet(header.fmtBitsPerSample);
    correctEndianAndSet(header.fmtCbSize);
    correctEndianAndSet(header.factChunkSize);


    mStream.write(reinterpret_cast<const char *>(&header), sizeof(header));
}

void Wav::finish() {


    uint32_t totalSamples = mSampleCount;
    uint32_t dataChunkSize = totalSamples * mChannels * 4;
    
    // chunk size totals
    // 4: riff chunk
    // 18 + 8: fmt chunk
    // 4 + 8: fact chunk
    // = 42
    // 8 + dataChunkSize + (0 or 1)
    uint32_t chunkSize = 50 + dataChunkSize;

    // do we need a pad byte?
    if (dataChunkSize & 1) {
        // yes, write a single 0 to the end of the data chunk
        uint8_t zero = 0;
        mStream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
        ++chunkSize;
    }

    // convert to little endian if needed
    correctEndianAndSet(chunkSize);
    correctEndianAndSet(totalSamples);
    correctEndianAndSet(dataChunkSize);

    // overwrite the chunk size for the entire file (also equal to the filesize - 8)
    mStream.seekp(offsetof(WavHeader, chunkSize));
    mStream.write(reinterpret_cast<const char *>(&chunkSize), sizeof(chunkSize));

    // overwrite the sample count in the fact subchunk
    mStream.seekp(offsetof(WavHeader, factSampleCount));
    mStream.write(reinterpret_cast<const char *>(&totalSamples), sizeof(totalSamples));

    // overwrite the chunk size of the data subchunk
    mStream.seekp(offsetof(WavHeader, dataChunkSize));
    mStream.write(reinterpret_cast<const char *>(&dataChunkSize), sizeof(dataChunkSize));



}

void Wav::write(float buf[], size_t nsamples) {

    size_t totalSamples = mChannels * nsamples;

    for (size_t i = 0; i != totalSamples; ++i) {
        float sample = correctEndian(buf[i]);
        mStream.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
        if (!mStream.good()) {
            return;
        }
    }

    mSampleCount += nsamples;
    
}



}
