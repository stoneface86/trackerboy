
#include "trackerboy/data/Instrument.hpp"

namespace trackerboy {

Instrument::Instrument() :
    mChannel(ChType::ch1),
    mEnvelopeEnabled(false),
    mEnvelope(0),
    mSequences()
{
}

Instrument::Instrument(Instrument const& instrument) :
    mChannel(instrument.mChannel),
    mEnvelopeEnabled(instrument.mEnvelopeEnabled),
    mEnvelope(instrument.mEnvelope),
    mSequences(instrument.mSequences)
{
}

ChType Instrument::channel() const noexcept {
    return mChannel;
}

bool Instrument::hasEnvelope() const noexcept {
    return mEnvelopeEnabled;
}

uint8_t Instrument::envelope() const noexcept {
    return mEnvelope;
}

std::optional<uint8_t> Instrument::queryEnvelope() const noexcept {
    if (mEnvelopeEnabled) {
        return mEnvelope;
    } else {
        return std::nullopt;
    }
}

Sequence::Enumerator Instrument::enumerateSequence(size_t parameter) const noexcept {
    return mSequences[parameter].enumerator();
}

Sequence& Instrument::sequence(size_t parameter) noexcept {
    return mSequences[parameter];
}

void Instrument::setChannel(ChType ch) noexcept {
    mChannel = ch;
}

void Instrument::setEnvelope(uint8_t envelope) noexcept {
    mEnvelope = envelope;
}

void Instrument::setEnvelopeEnable(bool enable) noexcept {
    mEnvelopeEnabled = enable;
}

}
