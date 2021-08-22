
#include "internal/fileformat/payload/handlers/InstHandler.hpp"
#include "internal/fileformat/payload/payload.hpp"
#include "internal/endian.hpp"
#include "internal/enumutils.hpp"

namespace trackerboy {

#define TU InstHandlerTU
namespace TU {

// The C++ standard does not require bool to be 1 byte, so we use a char, which
// is, instead.
using charbool = char;

#pragma pack(push, 1)

struct InstrumentFormat {
    uint8_t channel;
    charbool envelopeEnabled;
    uint8_t envelope;
    // sequence data follows
};

struct SequenceFormat {
    uint16_t length;
    charbool loopEnabled;
    uint8_t loopIndex;
    // sequence data follows uint8_t[length]
};

#pragma pack(pop)
}

FormatError InstHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)index;
    auto inst = initItem(block, mod.instrumentTable());
    if (inst == nullptr) {
        return FormatError::duplicateId;
    }
    return deserializeInstrument(block, *inst);
}

void InstHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)index;
    
    auto inst = nextItem(mod.instrumentTable());
    serializeItem(block, *inst);

    TU::InstrumentFormat format;
    format.channel = +inst->channel();
    format.envelopeEnabled = (TU::charbool)inst->hasEnvelope();
    format.envelope = inst->envelope();
    block.write(format);

    for (auto &sequence : inst->sequences()) {
        TU::SequenceFormat sequenceFmt;
        auto &seqdata = sequence.data();
        sequenceFmt.length = correctEndian((uint16_t)seqdata.size());
        auto loop = sequence.loop();
        sequenceFmt.loopEnabled = (TU::charbool)loop.has_value();
        sequenceFmt.loopIndex = loop.value_or((uint8_t)0);
        block.write(sequenceFmt);
        block.write(seqdata.size(), seqdata.data());
    }

}

FormatError InstHandler::deserializeInstrument(InputBlock &block, Instrument &inst) {
    TU::InstrumentFormat format;
    block.read(format);
    if (format.channel > +ChType::ch4) {
        return FormatError::unknownChannel;
    }
    inst.setChannel(static_cast<ChType>(format.channel));
    inst.setEnvelopeEnable(format.envelopeEnabled);
    inst.setEnvelope(format.envelope);

    for (size_t i = 0; i < Instrument::SEQUENCE_COUNT; ++i) {
        auto &sequence = inst.sequence(i);

        TU::SequenceFormat sequenceFmt;
        block.read(sequenceFmt);
        sequenceFmt.length = correctEndian(sequenceFmt.length);
        if (sequenceFmt.length > Sequence::MAX_SIZE) {
            return FormatError::invalid;
        }

        sequence.resize(sequenceFmt.length);
        if (sequenceFmt.loopEnabled) {
            sequence.setLoop(sequenceFmt.loopIndex);
        }
        auto &seqdata = sequence.data();
        block.read(sequenceFmt.length, seqdata.data());
    }
    return FormatError::none;
}

}

#undef TU
