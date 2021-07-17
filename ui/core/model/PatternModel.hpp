
#pragma once

#include "core/Module.hpp"

class OrderModel;
class SongModel;

#include "core/clipboard/PatternClip.hpp"
#include "core/PatternCursor.hpp"
#include "core/PatternSelection.hpp"

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

    enum CursorChangeFlag {
        CursorRowChanged = 0x1,
        CursorColumnChanged = 0x2,
        CursorTrackChanged = 0x4,
        CursorUnchanged = 0x0
    };
    Q_DECLARE_FLAGS(CursorChangeFlags, CursorChangeFlag)

    enum SelectMode {
        SelectionKeep,      // the current selection will be kept
        SelectionModify,    // the current selection will be modified
        SelectionRemove     // the current selection will be deselected
    };


    explicit PatternModel(Module &mod, OrderModel &orderModel, SongModel &songModel, QObject *parent = nullptr);
    virtual ~PatternModel() = default;

    void reload();

    // Data Access ============================================================

    // TODO: should probably make these const

    trackerboy::Pattern* previousPattern();

    trackerboy::Pattern& currentPattern();

    trackerboy::Pattern* nextPattern();

    // Properties =============================================================

    int cursorRow() const;

    int cursorColumn() const;

    int cursorTrack() const;

    PatternCursor cursor() const;

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
    // Gets the current selection.
    //
    PatternSelection selection() const;

    //
    // Sets the selection. If nothing is selected, then the item at the given
    // point is selected. If there is a selection, then the end point is set
    // to the given point.
    //
    void setSelection(PatternCursor pos);

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

    //
    // Selects the entire row
    //
    void selectRow(int row);

    //
    // Removes the current selection
    //
    void deselect();

    //
    // Gets a clip of the current selection or the cursor if there is no selection
    //
    PatternClip clip();

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

    // transpose the current note or selection by the given number of semitones
    void transpose(int amount);

    // reverses rows in a selection
    void reverse();

    // moves the selected data to a new position
    void moveSelection(PatternCursor pos);

    void paste(PatternClip const& clip, bool mix);

signals:
    void cursorChanged(CursorChangeFlags flags);

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

    void moveCursorRow(int amount, SelectMode mode = SelectionKeep);
    void moveCursorColumn(int amount, SelectMode mode = SelectionKeep);
    void moveCursorTrack(int amount);

    void setCursorRow(int row);
    void setCursorColumn(int column);
    void setCursorTrack(int track);

    void setCursor(PatternCursor const cursor);

    void setRecord(bool recording);

    void setFollowing(bool following);

    void setPreviewEnable(bool previews);

private slots:
    // connected to OrderModel's patternChanged signal
    void setCursorPattern(int pattern);

private:

    // QUndoCommand command classes
    friend class TrackEditCmd;
    friend class SelectionCmd;
    friend class DeleteSelectionCmd;
    friend class PasteCmd;
    friend class TransposeCmd;
    friend class ReverseCmd;

    Q_DISABLE_COPY(PatternModel)

    void setCursorRowImpl(int row, CursorChangeFlags &flags);
    void setCursorColumnImpl(int col, CursorChangeFlags &flags);
    void setCursorTrackImpl(int track, CursorChangeFlags &flags);
    void setCursorPatternImpl(int pattern, CursorChangeFlags &flags);

    void setPatterns(int pattern, CursorChangeFlags &flags);
    void setPreviewPatterns(int pattern);

    void emitIfChanged(CursorChangeFlags flags);

    int cursorEffectNo();

    trackerboy::TrackRow const& cursorTrackRow();

    void invalidate(int pattern, bool updatePatterns);

    bool selectionDataIsEmpty();

    Module &mModule;
    OrderModel &mOrderModel; // Temporary: break up OrderModel

    PatternCursor mCursor;
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
    PatternSelection mSelection;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(PatternModel::CursorChangeFlags)
