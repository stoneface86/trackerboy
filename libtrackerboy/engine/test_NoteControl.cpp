
#include "catch.hpp"

#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/note.hpp"


/*
Test scenarios

TD: trigger delay (0 means no trigger)
CD: cut delay (0 means no cut)


                | TD  | CD  | playing condition | playing pre-condition
[1] C-5 ... ... | 1   | 0   | 0                 | false
[2] C-5 Gxx ... | xx  | n/a | xx                | false
[3] --  ... ... | 0   | 1   | always false      | true
[4] --  Gxx ... | 0   | xx  | < xx              | true
[5] ... Sxx ... | 0   | xx  | < xx              | true
[6] --  Sxx ... | 0   | 1   | always false      | true
[7] C-5 Gxx Syy | xx  | yy  | xx >=, < yy       | false
[8] --  Gxx Syy | 0   | yy  | < yy              | true
[9] --  Syy Gxx | 0   | xx  | < xx              | true


Scenario descriptions
[1]: triggers C-5 immediately
[2]: triggers C-5 after xx frames
[3]: note cuts immediately
[4]: note cuts after xx frames
[5]: note cuts after xx frames
[6]: note cuts immediately
[7]: note triggers C-5 after xx frames, then cuts after yy frames
[8]: note cuts after yy frames
[9]: note cuts after xx frames

notes:
[4], [5] and [9] have equivalent behavior
A NOTE_CUT is converted to a delayed cut with a delay of the set note delay
--  ... ...  =>  ... S00 ...
--  G01 ...  =>  ... S01 ...
 order matters! last effect set is the effective one ([8] vs [9])
--  G03 S05  =>  ... S05 ...
--  S05 G03  =>  ... S03 ...

for [7]
if xx > yy: the previous note is cut first and then the next is triggered after xx frames
if xx < yy: the note is triggered first, then cut (ie only plays for yy - xx frames)
if xx = yy: the note triggered and cut at the same time, resulting in a cut
*/

namespace trackerboy {

namespace {

static constexpr uint8_t TEST_NOTE = NOTE_C + OCTAVE_5;

// invalid note index
static constexpr uint8_t ILLEGAL_NOTE = 0xFF;

static constexpr uint8_t DELAY = 3;
static constexpr uint8_t DELAY2 = 5;

}


TEST_CASE("behavior testing", "[NoteControl]") {

    NoteControl nc;

    SECTION("note triggers") {

        // pre-condition
        REQUIRE(!nc.isPlaying());

        SECTION("note triggers immediately [1]") {
            // C-5 ... ...
            nc.noteTrigger(TEST_NOTE, 0);
            REQUIRE(nc.step().has_value());
            REQUIRE(nc.isPlaying());
        }

        SECTION("note triggers after xx frames [2]") {
            // C-5 Gxx ...
            nc.noteTrigger(TEST_NOTE, DELAY);
            for (uint8_t i = 0; i != DELAY; ++i) {
                REQUIRE(!nc.isPlaying());
                REQUIRE(!nc.step().has_value());
            }

            REQUIRE(nc.step().has_value());
            REQUIRE(nc.isPlaying());

        }
    }

    SECTION("note cuts") {
        // pre-condition
        nc.noteTrigger(TEST_NOTE);
        REQUIRE(nc.step().has_value());
        REQUIRE(nc.isPlaying());

        SECTION("immediately") {

            SECTION("using special note NOTE_CUT [3]") {
                
            }

            SECTION("using special note NOTE_CUT and delayed cut [6]") {
                nc.noteCut(DELAY);
            }

            nc.noteTrigger(NOTE_CUT);
            REQUIRE(!nc.step().has_value());
            REQUIRE(!nc.isPlaying());
        }

        SECTION("after xx frames") {

            // behavior for [4], [5] and [9] are exactly the same
            // [4] uses Gxx (delayed note trigger)
            // [5] uses Sxx (delayed note cut)
            // [9] uses both, but Gxx is the last effect so that one is used

            SECTION("using note trigger [4]") {
                nc.noteTrigger(NOTE_CUT, DELAY);
            }

            SECTION("using note cut [5]") {
                nc.noteCut(DELAY);
            }

            SECTION("using both [9]") {
                nc.noteCut(DELAY + 5);
                nc.noteTrigger(NOTE_CUT, DELAY);
            }

            for (uint8_t i = 0; i != DELAY + 1; ++i) {
                REQUIRE(nc.isPlaying());
                REQUIRE(!nc.step().has_value());
            }
            REQUIRE(!nc.isPlaying());
        }

    }

    SECTION("note triggers and cuts [7]") {

        // pre-condition
        nc.noteTrigger(TEST_NOTE);
        REQUIRE(nc.step().has_value());
        REQUIRE(nc.isPlaying());


        SECTION("xx > yy") {
            // 0 1 2 3 4 5 6 7 8 9 ...
            // T T T T F F F F F T T
            // ^                 ^
            // trigger           trigger
            constexpr uint8_t ND = 8;
            constexpr uint8_t CD = 3;
            
            nc.noteTrigger(TEST_NOTE, ND);
            nc.noteCut(CD);

            for (uint8_t i = 0; i != CD; ++i) {
                REQUIRE(!nc.step());
                REQUIRE(nc.isPlaying());
            }

            for (uint8_t i = 0; i != ND - CD; ++i) {
                REQUIRE(!nc.step());
                REQUIRE(!nc.isPlaying());
            }

            REQUIRE(nc.step());
            REQUIRE(nc.isPlaying());


        }

        SECTION("xx < yy") {
            // 0 1 2 3 4 5 6 7 8 9 ...
            // T T T T T T T F F F
            //         ^
            constexpr uint8_t ND = 3;
            constexpr uint8_t CD = 6;

            nc.noteTrigger(TEST_NOTE, ND);
            nc.noteCut(CD);

            for (uint8_t i = 0; i != ND; ++i) {
                REQUIRE(!nc.step());
                REQUIRE(nc.isPlaying());
            }

            REQUIRE(nc.step());
            REQUIRE(nc.isPlaying());

            for (uint8_t i = 0; i != CD - ND - 1; ++i) {
                REQUIRE(!nc.step());
                REQUIRE(nc.isPlaying());
            }

            REQUIRE(!nc.step());
            REQUIRE(!nc.isPlaying());
        }

        SECTION("xx == yy") {
            // 0 1 2 3 4 5 6 7 8 9 ...
            // T T T T F F F F F F ...
            // no trigger for this one

            constexpr uint8_t ND = 3;
            constexpr uint8_t CD = ND;

            nc.noteTrigger(TEST_NOTE, ND);
            nc.noteCut(CD);

            for (uint8_t i = 0; i != ND; ++i) {
                REQUIRE(!nc.step());
                REQUIRE(nc.isPlaying());
            }

            REQUIRE(!nc.step());
            REQUIRE(!nc.isPlaying());
        }



    }

    SECTION("reset") {
        nc.noteTrigger(TEST_NOTE);
        nc.step();

        nc.reset();
        for (int i = 0; i != 256; ++i) {
            REQUIRE(!nc.isPlaying());
            nc.step();
        }
    }


}



}
