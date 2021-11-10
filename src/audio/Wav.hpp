/*
** Wav.hpp
**
** A header-only library for writing wav files. To use, define
** WAV_IMPLEMENTATION in one of your source files that include this header.
**
** To create a file, construct a Wav object with a filepath, number of channels
** and samplerate. Then write as many samples you want to it via the write
** method. Note that for multichannel data, the samples are interleaved.
**
** This library only writes wav files using 32-bit float samples.
**
** stoneface86
**
** ----------------------------------------------------------------------------
**
** This is free and unencumbered software released into the public domain.
**
** Anyone is free to copy, modify, publish, use, compile, sell, or
** distribute this software, either in source code form or as a compiled
** binary, for any purpose, commercial or non-commercial, and by any
** means.
**
** In jurisdictions that recognize copyright laws, the author or authors
** of this software dedicate any and all copyright interest in the
** software to the public domain. We make this dedication for the benefit
** of the public at large and to the detriment of our heirs and
** successors. We intend this dedication to be an overt act of
** relinquishment in perpetuity of all present and future rights to this
** software under copyright law.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
** OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
** OTHER DEALINGS IN THE SOFTWARE.
**
** For more information, please refer to <https://unlicense.org>
*/


#pragma once

#include <cstddef>
#include <fstream>
#include <string>


class Wav {

public:


    //
    // Opens a wav file for writing sample data with the given channel count
    // and samplerate. Existing files will be overwritten.
    //
    explicit Wav(std::string const& filename, int channels, int samplerate);

    //
    // Adjusts the wav header with the final number of samples written and
    // closes the file.
    //
    ~Wav();

    //
    // Get the stream used by the wav writer.
    //
    std::ofstream const& stream() const;

    //
    // Writes the given number of samples from the given buffer to the wav
    // file. The buffer should be at least the size of nsamples * channels.
    //
    void write(float buf[], std::size_t nsamples);

private:

    // non-copyable
    Wav(Wav const& wav) = delete;
    Wav& operator=(Wav const& wav) = delete;

    // non-movable
    Wav(Wav &&wav) = delete;
    Wav& operator=(Wav &&wav) = delete;

    std::ofstream mStream;
    std::size_t mSampleCount;

    int mChannels;
    int mSamplingRate;

};
