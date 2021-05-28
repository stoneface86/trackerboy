
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

    enum ColumnType {
        COLUMN_NOTE,

        // high is the upper nibble (bits 4-7)
        // low is the lower nibble (bits 0-3)

        COLUMN_INSTRUMENT_HIGH,
        COLUMN_INSTRUMENT_LOW,

        COLUMN_EFFECT1_TYPE,
        COLUMN_EFFECT1_ARG_HIGH,
        COLUMN_EFFECT1_ARG_LOW,

        COLUMN_EFFECT2_TYPE,
        COLUMN_EFFECT2_ARG_HIGH,
        COLUMN_EFFECT2_ARG_LOW,

        COLUMN_EFFECT3_TYPE,
        COLUMN_EFFECT3_ARG_HIGH,
        COLUMN_EFFECT3_ARG_LOW

    };

    explicit PatternModel(ModuleDocument &doc, QObject *parent = nullptr);

    void reload();

    trackerboy::Pattern* previousPattern();

    trackerboy::Pattern& currentPattern();

    trackerboy::Pattern* nextPattern();

    int cursorRow() const;

    int cursorColumn() const;

    ColumnType columnType() const;

    int trackerCursorRow() const;
    int trackerCursorPattern() const;

    bool isRecording() const;

    bool isFollowing() const;

    bool isPlaying() const;

    void setTrackerCursor(int row, int pattern);
    void setPlaying(bool playing);


    // editing

    //
    // sets the note for the current track at the cursor row. An empty optional
    // deletes the note set.
    //
    void setNote(std::optional<uint8_t> note);
    void setInstrument(std::optional<uint8_t> nibble);
    void setEffectType(trackerboy::EffectType type);
    void setEffectParam(uint8_t nibble);

    // deletes selection or the cursor if no selection is present
    void deleteSelection();

signals:
    void cursorColumnChanged(int column);
    void cursorRowChanged(int row);
    void patternsChanged();
    void patternSizeChanged(int rows);
    void trackerCursorChanged(int row, int pattern);
    void playingChanged(bool playing);
    void recordingChanged(bool recording);

    void dataChanged();

public slots:

    void moveCursorRow(int amount);
    void moveCursorColumn(int amount);

    void setCursorRow(int row);
    void setCursorColumn(int column);

    void setRecord(bool recording);

    void setFollowing(bool following);

    void setPreviewEnable(bool previews);

private slots:
    // connected to OrderModel's patternChanged signal
    void setCursorPattern(int pattern);
    void setCursorTrack(int track);

private:
    friend class PatternEditColumnCmd;
    friend struct ColumnEditHelper;

    Q_DISABLE_COPY(PatternModel)

    void setPatterns(int pattern);
    void setPreviewPatterns(int pattern);

    int cursorEffectNo();

    ModuleDocument &mDocument;

    int mCursorRow;
    int mCursorColumn;
    int mCursorPattern;

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

