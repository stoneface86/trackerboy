
#include "catch.hpp"
#include "./Order.cpp"
// fix this later
#include "../fileformat.cpp"

using namespace trackerboy;

static constexpr size_t SAMPLE_ORDER_SIZE = 10;

static void setIndexVec(std::vector<uint8_t> &vec) {
    for (size_t i = 0; i != SAMPLE_ORDER_SIZE; ++i) {
        vec.push_back(static_cast<uint8_t>(i));
    }
}

static void testSequenceFor(Order::Sequencer &seq, size_t loopIndex, size_t runs, bool seqShouldContinue) {
    size_t start = 0;
    do {
        for (size_t i = start; i != SAMPLE_ORDER_SIZE; ++i) {
            CHECK(seq.hasNext());
            CHECK(seq.next() == static_cast<uint8_t>(i));
        }
        start = loopIndex;
    } while (runs--);

    // check hasNext is the expected value
    CHECK(seq.hasNext() == seqShouldContinue);
}


TEST_CASE("setLoop throws exception when index out of range", "[order]") {
    

    SECTION("empty index vector") {
        Order ord;
        CHECK_THROWS_AS(ord.setLoop(0), std::out_of_range);
    }

    
    SECTION("sample index vector") {
        Order ord;
        setIndexVec(ord.indexVec());

        CHECK_NOTHROW(ord.setLoop(0));
        CHECK_NOTHROW(ord.setLoop(SAMPLE_ORDER_SIZE - 1));
        CHECK_THROWS_AS(ord.setLoop(SAMPLE_ORDER_SIZE), std::out_of_range);
    }
}


TEST_CASE("Sequencer testing", "[order]") {

    Order ord;

    SECTION("default sequencer hasNext() is false") {
        Order::Sequencer seq(ord);

        CHECK(seq.hasNext() == false);
    }


    SECTION("sequencer with sample order") {
        setIndexVec(ord.indexVec());
        
        SECTION("sequence runs once") {
            Order::Sequencer seq(ord);
            testSequenceFor(seq, 0, 0, false);
        }
        
        SECTION("sequence with loop counter runs once") {
            Order::Sequencer seq(ord, 5);
            testSequenceFor(seq, 0, 0, false); // the sequence runs once since no loop index was set
        }

    }

    SECTION("sequencer with sample order and loop index set") {
        setIndexVec(ord.indexVec());
        constexpr uint8_t SAMPLE_LOOP_INDEX = 5;
        constexpr uint8_t SAMPLE_RUN_FOR = 5;
        ord.setLoop(SAMPLE_LOOP_INDEX);

        SECTION("sequence runs forever") {
            Order::Sequencer seq(ord);
            // sequence runs forever, hasNext will always be true
            testSequenceFor(seq, SAMPLE_LOOP_INDEX, SAMPLE_RUN_FOR, true); 
        }

        SECTION("sequence runs for fixed number of loops") {
            Order::Sequencer seq(ord, SAMPLE_RUN_FOR);
            testSequenceFor(seq, SAMPLE_LOOP_INDEX, SAMPLE_RUN_FOR, false);
        }
    }

    



}

