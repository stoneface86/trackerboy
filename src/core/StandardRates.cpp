
#include "core/StandardRates.hpp"
#include <QtGlobal>

#include <array>

#define TU sampleratesTU
namespace TU {

static std::array const RATES = {
    11025,
    12000,
    22050,
    24000,
    44100,
    48000,
    96000
};

}

namespace StandardRates {

static_assert(TU::RATES.size() == COUNT, "samplerate table does not match size");

int get(size_t index) noexcept {
    Q_ASSERT(index < COUNT);
    return TU::RATES[index];
}

}

#undef TU
