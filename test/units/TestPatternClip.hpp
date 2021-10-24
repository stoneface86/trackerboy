
#include <QtTest/QtTest>
#include "core/clipboard/PatternClip.hpp"

#include "trackerboy/data/PatternMap.hpp"
#include "trackerboy/note.hpp"

#include <QMimeData>

#include <array>

class TestPatternClip : public QObject {

    Q_OBJECT

    // sample data to use during testing
    trackerboy::Track mCh1Track;
    trackerboy::Track mCh4Track;
    trackerboy::Track mEmptyTrack;

public:

    Q_INVOKABLE TestPatternClip(QObject *parent = nullptr);

private slots:
    // test cases

    void defaultConstructor();

    void copying();

    void moving();

    void overwritePaste();

    void mixPaste();

    void badMime();

    void persistance();


private:

    // creates copies of the given tracks for a pattern
    // so that test cases can modify this copy, leaving the original tracks untouched
    class PatternCopy {

    public:
        PatternCopy(trackerboy::Track const& tr1, trackerboy::Track const& tr2, trackerboy::Track const& tr3, trackerboy::Track const& tr4);

        trackerboy::Pattern pattern();

        trackerboy::Track& operator[](size_t i);

    private:
        std::array<trackerboy::Track, 4> mTracks;

    };

    trackerboy::Pattern emptyPattern();

    trackerboy::Pattern samplePattern();


};
