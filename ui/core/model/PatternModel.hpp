
#pragma once

class ModuleDocument;

#include "trackerboy/data/PatternRow.hpp"
#include "trackerboy/data/Pattern.hpp"

#include <QObject>
#include <QRect>

#include <optional>

//
// Model class for accessing/modifying pattern data for a song.
//
class PatternModel : public QObject {

    Q_OBJECT

public:

    //
    // enum for editable columns
    //
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

    //
    // Selectable columns
    //
    enum SelectType {
        SelectNote,
        SelectInstrument,
        SelectEffect1,
        SelectEffect2,
        SelectEffect3
    };

    explicit PatternModel(ModuleDocument &doc, QObject *parent = nullptr);

    void reload();

    // Data Access ============================================================

    trackerboy::Pattern* previousPattern();

    trackerboy::Pattern& currentPattern();

    trackerboy::Pattern* nextPattern();

    // Properties =============================================================

    int cursorRow() const;

    int cursorColumn() const;

    ColumnType columnType() const;

    SelectType selectType() const;

    int trackerCursorRow() const;
    int trackerCursorPattern() const;

    bool isRecording() const;

    bool isFollowing() const;

    bool isPlaying() const;

    void setTrackerCursor(int row, int pattern);
    void setPlaying(bool playing);

    // Selection ==============================================================

    //
    // Returns true if there is a selection set, false otherwise.
    //
    bool hasSelection() const;

    //
    // Gets the current selection. QRect is used to model the selection area.
    // The x-coordinate of the rectangle is the data column and the y-coordinate
    // is the row. The returned rect is normalized, or the top-left corner is
    // less than the bottom-right. If there is no selection the returned rect
    // is null (ie QRect::isNull() returns true). 
    //
    QRect selection() const;

    //
    // Sets the selection. If nothing is selected, then the item at the given
    // point is selected. If there is a selection, then the end point is set
    // to the given point.
    //
    void setSelection(QPoint const point);

    //
    // Calls setSelection with the current cursor position.
    //
    void selectCursor();

    //
    // Selects entire the entire track or the entire pattern. The track is
    // selected first. If the function is called again with the track selected
    // then the entire pattern is selected. Therefore, repeatedly calling this
    // function will alternate between selecting the track and pattern.
    //
    void selectAll();

    void selectRow(int row);

    //
    // Removes the current selection
    //
    void deselect();

    // Editing ================================================================

    // unless specified, these functions use the current cursor position for
    // editing.

    //
    // sets the note for the current track at the cursor row. An empty optional
    // deletes the note set. If an instrument is provided, it will be set along
    // with the note
    //
    void setNote(std::optional<uint8_t> note, std::optional<uint8_t> instrument);
    void setInstrument(std::optional<uint8_t> nibble);
    void setEffectType(trackerboy::EffectType type);
    void setEffectParam(uint8_t nibble);

    // deletes selection or the cursor if no selection is present
    void deleteSelection();

signals:
    void cursorColumnChanged(int column);
    void cursorRowChanged(int row);
    //void patternsChanged();
    void patternSizeChanged(int rows);
    void trackerCursorChanged(int row, int pattern);
    void playingChanged(bool playing);
    void recordingChanged(bool recording);

    void selectionChanged();

    //
    // emitted when a change has been made to the current pattern and should
    // be redrawn.
    //
    void invalidated();

public slots:

    void moveCursorRow(int amount, bool select = false);
    void moveCursorColumn(int amount, bool select = false);

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

    friend class TrackEditCmd;
    friend class DeleteSelectionCmd;

    Q_DISABLE_COPY(PatternModel)

    void setPatterns(int pattern);
    void setPreviewPatterns(int pattern);

    int cursorEffectNo();

    trackerboy::TrackRow const& cursorTrackRow();

    void invalidate(int pattern, bool updatePatterns);

    bool selectionDataIsEmpty();

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

    bool mHasSelection;
    QRect mSelection;

public:

    // constants

    static constexpr auto COLUMNS_PER_TRACK = 12;

    static constexpr auto COLUMNS = COLUMNS_PER_TRACK * 4;

    static constexpr auto SELECTS_PER_TRACK = 5;
    static constexpr auto SELECTS = SELECTS_PER_TRACK * 4;

};

