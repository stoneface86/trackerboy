
#include <QtTest/QtTest>
#include "core/clipboard/PatternClip.hpp"

#include "trackerboy/data/PatternMap.hpp"
#include "trackerboy/note.hpp"

#include <QMimeData>

#include <array>

constexpr auto PATTERN_SIZE = 8;

class TestPatternClip : public QObject {

    Q_OBJECT

    // sample data to use during testing
    trackerboy::Track mCh1Track;
    trackerboy::Track mCh4Track;
    trackerboy::Track mEmptyTrack;

public:

    TestPatternClip(QObject *parent = nullptr) :
        QObject(parent),
        mCh1Track(PATTERN_SIZE),
        mCh4Track(PATTERN_SIZE),
        mEmptyTrack(PATTERN_SIZE)
    {

        // sample pattern data
        //      mCh1Track    mEmptyTrack  mEmptyTrack  mCh4Track
        // 00 | G-5 00 ... | ... .. ... | ... .. ... | G-6 01 ... |
        // 01 | ... .. ... | ... .. ... | ... .. ... | ... .. ... |
        // 02 | ... .. ... | ... .. ... | ... .. ... | G-6 01 G03 |
        // 03 | ... .. ... | ... .. ... | ... .. ... | ... .. ... |
        // 04 | B-5 00 ... | ... .. ... | ... .. ... | G-6 02 ... |
        // 05 | ... .. ... | ... .. ... | ... .. ... | ... .. ... |
        // 06 | ... .. ... | ... .. ... | ... .. ... | G-6 01 G03 |
        // 07 | ... .. ... | ... .. ... | ... .. ... | ... .. ... |


        mCh1Track.setNote(0, trackerboy::NOTE_G + trackerboy::OCTAVE_5);
        mCh1Track.setInstrument(0, 0);
        mCh1Track.setNote(4, trackerboy::NOTE_B + trackerboy::OCTAVE_5);
        mCh1Track.setInstrument(4, 0);

        mCh4Track.setNote(0, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        mCh4Track.setInstrument(0, 1);
        mCh4Track.setNote(2, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        mCh4Track.setInstrument(2, 1);
        mCh4Track.setEffect(2, 0, trackerboy::EffectType::delayedNote, 3);

        mCh4Track.setNote(4, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        mCh4Track.setInstrument(4, 2);
        mCh4Track.setNote(6, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        mCh4Track.setInstrument(6, 1);
        mCh4Track.setEffect(6, 0, trackerboy::EffectType::delayedNote, 3);
    }

private slots:
    // test cases

    void defaultConstructor() {

        PatternClip clip;
        QVERIFY(!clip.hasData());

    }

    void copying() {
        PatternClip clip;
        clip.save(samplePattern(), PatternSelection(PatternAnchor(0, 2, 1), PatternAnchor(6, 0, 3)));

        QVERIFY(clip.hasData());

        PatternClip copy;
        copy = clip;

        QVERIFY(copy.hasData());
        QVERIFY(clip.hasData());
        QVERIFY(copy == clip);
    }

    void moving() {

        PatternClip clipMovedFrom;
        clipMovedFrom.save(samplePattern(), PatternSelection(PatternAnchor(0, 2, 1), PatternAnchor(6, 0, 3)));

        QVERIFY(clipMovedFrom.hasData());

        PatternClip clipMovedTo(std::move(clipMovedFrom));

        // ignore clang-analyzer warnings about using a moved-from object
        QVERIFY(!clipMovedFrom.hasData());
        QVERIFY(clipMovedTo.hasData());
        QVERIFY(clipMovedFrom != clipMovedTo);

    }

    void overwritePaste() {
        // overwrite paste all of track 1 from our sample pattern to track 4

        // acquire the clip (all of CH1)
        PatternClip clip;
        PatternCopy patternData(mCh1Track, mEmptyTrack, mEmptyTrack, mCh4Track);
        auto pattern = patternData.pattern();
        clip.save(pattern,
            PatternSelection(
                PatternAnchor(0, 0, 0),
                PatternAnchor(PATTERN_SIZE - 1, PatternAnchor::MAX_SELECTS - 1, 0)
            )
        );


        // overwrite paste at CH4
        clip.paste(pattern, PatternCursor(0, 0, 3), false);

        // verify the result of the paste
        QVERIFY(patternData[0] == mCh1Track);
        QVERIFY(patternData[1] == mEmptyTrack);
        QVERIFY(patternData[2] == mEmptyTrack);
        QVERIFY(patternData[3] == mCh1Track);




    }

    void mixPaste() {

        // acquire the clip (all of CH1)
        PatternClip clip;
        PatternCopy patternData(mCh1Track, mEmptyTrack, mEmptyTrack, mCh4Track);
        auto pattern = patternData.pattern();
        clip.save(pattern,
            PatternSelection(
                PatternAnchor(0, 0, 0),
                PatternAnchor(PATTERN_SIZE - 1, PatternAnchor::MAX_SELECTS - 1, 0)
            )
        );

        // mix paste at CH4
        // pasting at CH4, row 0, note column results in no change to the pattern
        clip.paste(pattern, PatternCursor(0, 0, 3), true);

        QVERIFY(patternData[0] == mCh1Track);
        QVERIFY(patternData[1] == mEmptyTrack);
        QVERIFY(patternData[2] == mEmptyTrack);
        QVERIFY(patternData[3] == mCh4Track);

        // now mix paste at row 1:
        // 00 ... | G-6 01 ... |     ... | G-6 01 ... |
        // 01 ... | ... .. ... |     ... | G-5 00 ... |
        // 02 ... | G-6 01 G03 |     ... | G-6 01 G03 |
        // 03 ... | ... .. ... |  => ... | ... .. ... |
        // 04 ... | G-6 02 ... |     ... | G-6 02 ... |
        // 05 ... | ... .. ... |     ... | B-5 00 ... |
        // 06 ... | G-6 01 G03 |     ... | G-6 01 G03 |
        // 07 ... | ... .. ... |     ... | ... .. ... |

        clip.paste(pattern, PatternCursor(1, 0, 3), true);
        QVERIFY(patternData[0] == mCh1Track);
        QVERIFY(patternData[1] == mEmptyTrack);
        QVERIFY(patternData[2] == mEmptyTrack);
        // create an expected track
        trackerboy::Track expected(PATTERN_SIZE);
        expected.setNote(1, trackerboy::NOTE_G + trackerboy::OCTAVE_5);
        expected.setInstrument(1, 0);
        expected.setNote(5, trackerboy::NOTE_B + trackerboy::OCTAVE_5);
        expected.setInstrument(5, 0);

        expected.setNote(0, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        expected.setInstrument(0, 1);
        expected.setNote(2, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        expected.setInstrument(2, 1);
        expected.setEffect(2, 0, trackerboy::EffectType::delayedNote, 3);

        expected.setNote(4, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        expected.setInstrument(4, 2);
        expected.setNote(6, trackerboy::NOTE_G + trackerboy::OCTAVE_6);
        expected.setInstrument(6, 1);
        expected.setEffect(6, 0, trackerboy::EffectType::delayedNote, 3);

        QVERIFY(patternData[3] == expected);
    }

    void badMime() {
        // test that fromMime returns false on invalid clip data

        // case 0: mime data with no data whatsoever
        {
            QMimeData data;
            PatternClip clip;
            QVERIFY(!clip.fromMime(&data));
        }

        // case 1: incorrect mime type
        {
            QMimeData data;
            data.setData("text/plain", "hello I am not a PatternClip");

            PatternClip clip;
            QVERIFY(!clip.fromMime(&data));
        }

        // cases 2 and 3 will mutate a QMimeData from sample data
        // and will expect fromMime to fail

        // get sample mime
        // save a sample clip of the sample pattern data
        QMimeData mime;
        {
            PatternClip clip;
            clip.save(samplePattern(), PatternSelection(PatternAnchor(0, 2, 1), PatternAnchor(6, 0, 3)));

            clip.toMime(&mime);
        }

        // case 2: correct mime type but data buffer is too small
        {
            QMimeData badMime;
            auto data = mime.data(PatternClip::MIME_TYPE);
            // truncate the data
            data.resize(sizeof(PatternSelection) - 1);

            badMime.setData(PatternClip::MIME_TYPE, data);

            PatternClip clip;
            QVERIFY(!clip.fromMime(&badMime));
        }

        // case 3: correct mime type but data buffer size does not match the clip region
        {
            QMimeData badMime;
            auto data = mime.data(PatternClip::MIME_TYPE);

            // buffer is too big
            data.resize(data.size() + 10);
            badMime.setData(PatternClip::MIME_TYPE, data);

            PatternClip clip;
            QVERIFY(!clip.fromMime(&badMime));

            // now try a smaller buffer
            badMime.clear();
            data.resize(data.size() - 20);
            badMime.setData(PatternClip::MIME_TYPE, data);
            QVERIFY(!clip.fromMime(&badMime));

        }

    }

    void persistance() {
        // test that saving a clip to MIME and reading a clip from the saved MIME data
        // are equivalent

        // save a sample clip of the sample pattern data
        PatternClip clip;
        clip.save(samplePattern(), PatternSelection(PatternAnchor(0, 2, 1), PatternAnchor(6, 0, 3)));

        QMimeData mime;
        clip.toMime(&mime);

        PatternClip clip2;
        QVERIFY(clip2.fromMime(&mime));

        // test that both clips are equal
        QVERIFY(clip == clip2);
    }


private:

    // creates copies of the given tracks for a pattern
    // so that test cases can modify this copy, leaving the original tracks untouched
    class PatternCopy {

    public:
        PatternCopy(trackerboy::Track const& tr1, trackerboy::Track const& tr2, trackerboy::Track const& tr3, trackerboy::Track const& tr4) :
            mTracks{tr1, tr2, tr3, tr4}
        {
        }

        trackerboy::Pattern pattern() {
            return {mTracks[0], mTracks[1], mTracks[2], mTracks[3]};
        }

        trackerboy::Track& operator[](size_t i) {
            return mTracks[i];
        }




    private:
        std::array<trackerboy::Track, 4> mTracks;

    };

    trackerboy::Pattern emptyPattern() {
        return { mEmptyTrack, mEmptyTrack, mEmptyTrack, mEmptyTrack };
    }

    trackerboy::Pattern samplePattern() {
        return { mCh1Track, mEmptyTrack, mEmptyTrack, mCh4Track };
    }


};



QTEST_MAIN(TestPatternClip)
#include "TestPatternClip.moc"
