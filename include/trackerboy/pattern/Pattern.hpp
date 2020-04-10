#pragma once

#include <fstream>
#include <iterator>
#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/EffectType.hpp"
#include "trackerboy/pattern/Track.hpp"
#include "trackerboy/pattern/TrackRow.hpp"


namespace trackerboy {


// Pattern data is stored in the PatternMaster and is accessed
// via Track objects. So a Pattern is just a bunch of Track objects
// for each channel
struct Pattern {


    Track track1;
    Track track2;
    Track track3;
    Track track4;

};



}
