
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "catch.hpp"

#include <array>

using namespace trackerboy;

bool operator==(ChannelState const& lhs, ChannelState const& rhs) {
    return lhs.playing == rhs.playing &&
        lhs.envelope == rhs.envelope &&
        lhs.panning == rhs.panning &&
        lhs.timbre == rhs.timbre &&
        lhs.frequency == rhs.frequency;
}

void noopTest(InstrumentRuntime &ir) {
    ChannelState defaultState(ChType::ch1);
    ChannelState state = defaultState;

    for (size_t i = 0; i != 5; ++i) {
        ir.step(state);
        CHECK((state == defaultState));
    }
}

void behaviorTest(InstrumentRuntime &ir, ChannelState expected[], size_t expectedSize, ChannelState initState) {
    auto state = initState;
    for (size_t i = 0; i != expectedSize; ++i) {
        ir.step(state);
        CHECK((state == expected[i]));
    }
}



TEST_CASE("behavior tests", "[InstrumentRuntime]") {

    Instrument inst;


        

    SECTION("runtime sets envelope") {

        constexpr uint8_t ENVELOPE = 0xA7;

        inst.setEnvelope(ENVELOPE);
        inst.setEnvelopeEnable(true);
        InstrumentRuntime ir(inst);

        ChannelState state(ChType::ch1);
        // expected state is the same as init, except that the envelope changed
        ChannelState expected = state;
        expected.envelope = ENVELOPE;

        ir.step(state);
        CHECK((state == expected));
        state = ChannelState();
        // envelope should only be set once
        ir.step(state);
        CHECK((state == ChannelState()));


    }

    SECTION("instrument with sequences") {

        std::array SEQUENCE_TEST = {
            ChannelState(false, 0x00, 0x00, 0x00, 0x00),
            ChannelState(false, 0x00, 0x01, 0x00, 0x00),
            ChannelState(false, 0x00, 0x02, 0x00, 0x00),
            ChannelState(false, 0x00, 0x03, 0x00, 0x00),
            ChannelState(false, 0x00, 0x00, 0x00, 0x00),
        };

        auto &seq = inst.sequence(Instrument::SEQUENCE_TIMBRE);
        auto &data = seq.data();
        // create the sequence from our expected test results
        for (auto &state : SEQUENCE_TEST) {
            data.push_back(state.timbre);
        }
        
        InstrumentRuntime ir(inst);

        behaviorTest(ir, SEQUENCE_TEST.data(), SEQUENCE_TEST.size(), ChannelState());
            
            
    }
        
   

}
