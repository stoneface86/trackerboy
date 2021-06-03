
#pragma once

#include "core/PatternSelection.hpp"

#include "trackerboy/data/Pattern.hpp"

#include <memory>

//
// Container class for clipped pattern data. A PatternSelection can be used to
// store a copy of the pattern data within that selection and can be restored
//
class PatternClip {

public:
    explicit PatternClip();

    bool hasData() const;

    PatternSelection const& selection();

    void restore(trackerboy::Pattern &dest);

    void save(trackerboy::Pattern const& src, PatternSelection region);


private:

    //
    // Moves data between a pattern and the clip. If saving = true, then
    // pattern data is copied to the clip buffer.
    //
    template <bool saving>
    void move(trackerboy::Pattern &pattern);

    std::unique_ptr<char[]> mData;
    PatternSelection mLocation;


};
