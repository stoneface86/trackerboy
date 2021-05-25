
#pragma once

class ModuleDocument;

#include "trackerboy/data/PatternRow.hpp"
#include "trackerboy/data/Pattern.hpp"

#include <QObject>

#include <optional>

//
// Model class for accessing/modifying pattern data for a song.
//
class PatternModel : public QObject {

    Q_OBJECT

public:

    explicit PatternModel(ModuleDocument &doc, QObject *parent = nullptr);

    std::optional<trackerboy::PatternRow> getRow(int index);

    int cursorRow() const;

    int cursorColumn() const;

    //int track() const;

    bool isRecording() const;

    void setTrackerCursor(int row, int pattern);
    void setPlaying(bool playing);

signals:
    void cursorColumnChanged(int column);
    void cursorRowChanged(int row);
    void patternsChanged();
    void patternSizeChanged(int rows);
    void trackerCursorChanged(int row, int pattern);
    void playingChanged(bool playing);
    void recordingChanged(bool recording);

public slots:

    void moveCursorRow(int amount);
    void moveCursorColumn(int amount);

    void setCursorRow(int row);
    void setCursorColumn(int column);

    void setRecord(bool recording);

    void setPreviewEnable(bool previews);

private slots:
    // connected to OrderModel's patternChanged signal
    void setCursorPattern(int pattern);
    void setCursorTrack(int track);

private:

    void setPreviewPatterns(int pattern);

    ModuleDocument &mDocument;

    int mCursorRow;
    int mCursorColumn;

    bool mRecording;
    bool mFollowing;
    bool mPlaying;
    bool mShowPreviews;

    int mTrackerRow;
    int mTrackerPattern;

    std::optional<trackerboy::Pattern> mPatternPrev;
    trackerboy::Pattern mPatternCurr;
    std::optional<trackerboy::Pattern> mPatternNext;

public:

    // constants

    static constexpr auto COLUMNS_PER_TRACK = 12;

    static constexpr auto COLUMNS = COLUMNS_PER_TRACK * 4;


};

