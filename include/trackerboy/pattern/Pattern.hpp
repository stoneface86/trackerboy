#pragma once

#include <tuple>

#include "trackerboy/pattern/Track.hpp"


namespace trackerboy {

//
// A pattern is a collective of Tracks, one for each channel
//
using Pattern = std::tuple<Track&, Track&, Track&, Track&>;

}
