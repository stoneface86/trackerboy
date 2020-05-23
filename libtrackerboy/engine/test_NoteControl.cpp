
#include "catch.hpp"

#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/note.hpp"


/*
Test scenarios

TD: trigger delay (n/a for no trigger)
CD: cut delay (n/a for no cut)


                | TD  | CD  | playing condition | playing pre-condition
[1] C-5 ... ... | 0   | n/a | 0                 | false
[2] C-5 Gxx ... | xx  | n/a | xx                | false
[3] --  ... ... | 0   | n/a | always false      | true
[4] --  Gxx ... | xx  | n/a | < xx              | true
[5] ... Sxx ... | n/a | xx  | < xx              | true
[6] --  Sxx ... | 0   | xx  | always false      | true
[7] C-5 Gxx Syy | xx  | yy  | xx >=, < yy       | false
[8] --  Gxx Syy | xx  | yy  | < xx              | true
[9] ??? ... ... | 0   | n/a | always false      | don't care
[A] ??? Gxx ... | xx  | n/a | always false
[B] ???

Scenario descriptions
[1]: triggers C-5 immediately
[2]: triggers C-5 after xx frames
[3]: note cuts immediately
[4]: note cuts after xx frames
[5]: note cuts after xx frames
[6]: note cuts immediately
[7]: note triggers C-5 after xx frames, then cuts after yy frames
[8]: note cuts after xx frames
[A]: illegal note, no delay
[B]: illegal note after xx frames

notes:
[4] and [5] have equivalent behavior
note triggers take priority over cuts, [8] will always cut at xx and [6] will always cut immediately

for [7]
if xx > yy: the note is cut first and then triggered
if xx < yy: the note is triggered first, then cut (ie only plays for yy - xx frames)
if xx = yy: the note triggered and cut at the same time, resulting in a cut
*/

namespace trackerboy {

namespace {

static constexpr uint8_t TEST_NOTE = NOTE_C + OCTAVE_5;

// invalid note index
static constexpr uint8_t ILLEGAL_NOTE = 0xFF;

static constexpr uint8_t DELAY = 3;

}


TEST_CASE("behavior testing", "[NoteControl]") {

    NoteControl nc;

    SECTION("note triggers") {

        // pre-condition
        REQUIRE(!nc.isPlaying());

        SECTION("note triggers immediately [1]") {
            // C-5 ... ...
            nc.noteTrigger(TEST_NOTE, 0);
            nc.step();
            REQUIRE(nc.isPlaying());
        }

        SECTION("note triggers after xx frames [2]") {
            // C-5 Gxx ...
            nc.noteTrigger(TEST_NOTE, DELAY);
            for (uint8_t i = 0; i != DELAY + 1; ++i) {
                REQUIRE(!nc.isPlaying());
                nc.step();
            }
            REQUIRE(nc.isPlaying());

        }
    }

    SECTION("note cuts") {
        // pre-condition
        nc.noteTrigger(TEST_NOTE);
        nc.step();
        REQUIRE(nc.isPlaying());

        SECTION("immediately") {

            nc.noteTrigger(NOTE_CUT);
            SECTION("using special note NOTE_CUT [3]") {
                
            }

            SECTION("using special note NOTE_CUT and delayed cut [6]") {
                nc.noteCut(DELAY);
            }

            nc.step();
            REQUIRE(!nc.isPlaying());
        }

        SECTION("after xx frames") {

            // behavior for [4] and [5] are exactly the same
            // [4] uses Gxx (delayed note trigger)
            // [5] uses Sxx (delayed note cut)

            SECTION("using note trigger [4]") {
                nc.noteTrigger(NOTE_CUT, DELAY);
            }

            SECTION("using note cut [5]") {
                nc.noteCut(DELAY);
            }

            for (uint8_t i = 0; i != DELAY + 1; ++i) {
                REQUIRE(nc.isPlaying());
                nc.step();
            }
            REQUIRE(!nc.isPlaying());
        }

    }

    SECTION("reset") {
        nc.noteTrigger(TEST_NOTE);
        nc.step();

        nc.reset();
        REQUIRE(nc.note() == 0);
        for (int i = 0; i != 256; ++i) {
            REQUIRE(!nc.isPlaying());
            nc.step();
        }
    }


}



}
