
#include "trackerboy/instrument.hpp"


namespace trackerboy {

Instrument::Instrument(uint8_t id, TrackId track) :
    id(id),
    trackId(track),
    program()
{
}


uint8_t Instrument::getId() {
    return id;
}


TrackId Instrument::getTrackId() {
    return trackId;
}


std::vector<Instruction>& Instrument::getProgram() {
    return program;
}

}