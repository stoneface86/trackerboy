
#pragma once

#include <cstdint>
#include "trackerboy/ChType.hpp"
#include "trackerboy/data/DataItem.hpp"


namespace trackerboy {


// Instruments are just a combination of effect settings that get
// applied on note trigger, used as a convenience for the composer




class Instrument : public DataItem {

public:

    #pragma pack(push, 1)
    struct Data {
        uint8_t timbre;         // V0x effect
        uint8_t envelope;       // Exx
        uint8_t panning;        // Ixy
        uint8_t delay;          // 0 for no delay (Gxx)
        uint8_t duration;       // 0 for infinite duration (Sxx)
        int8_t tune;            // Pxx
        uint8_t vibrato;        // 4xy
        uint8_t vibratoDelay;   // 5xx
    };
    #pragma pack(pop)

    Instrument();

    Data& data();

protected:
    virtual bool serializeData(std::ostream &stream) noexcept override;
    virtual bool deserializeData(std::istream &stream) noexcept override;

private:
    Data mData;
};


}
