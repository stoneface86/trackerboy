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

#include <cstdint>
#include <optional>
#include <vector>

namespace trackerboy {

//
// A sequence is a list of bytes that modulate an instrument parameter, such as
// duty or pitch.
//
class Sequence {

public:

    static constexpr size_t MAX_SIZE = 256;

    //
    // class for enumerating the values of a sequence
    //
    class Enumerator {

    public:
        bool hasNext();

        std::optional<uint8_t> next();

    private:
        friend class Sequence;
        Enumerator(Sequence const& sequence);

        Sequence const& mSequence;
        uint8_t mIndex;
    };

    Sequence();
    Sequence(Sequence const& seq);

    std::vector<uint8_t>& data() noexcept;
    std::vector<uint8_t> const& data() const noexcept;

    Enumerator enumerator() const;

    void resize(size_t size);

    std::optional<uint8_t> loop() const noexcept;

    void setLoop(uint8_t loop);

    void removeLoop();



private:

    // sequence data
    std::vector<uint8_t> mData;
    // index of the loop point, when end of sequence is reached it will loop back to this index
    std::optional<uint8_t> mLoop;

};


}
