
#pragma once

#include "graphics/PatternLayout.hpp"
#include "graphics/PatternPainter.hpp"
#include "model/PatternModel.hpp"
#include "config/data/Palette.hpp"
#include "config/data/PianoInput.hpp"
#include "widgets/grid/PatternGridHeader.hpp"

#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/note.hpp"

#include <QWidget>
#include <QPaintEvent>
#include <QString>
#include <QRect>
#include <QSize>

#include <optional>


class PatternGrid : public QWidget {

    Q_OBJECT

public:

    PatternGrid(
        PatternGridHeader &header,
        PatternModel &model,
        QWidget *parent = nullptr
    );
    virtual ~PatternGrid();

    void setColors(Palette const& pal);

    // Settings

    // show pattern data for the previous and next patterns
    //void setPreviewEnable(bool previews);

    // if true then flats will be shown instead of sharps
    void setShowFlats(bool showFlats);

    //
    // If this setting is true, a shadow is drawn from under the grid header.
    //
    void setShowShadow(bool shadow);

    //
    // If this setting is true, row numbers will be drawn in hexadecimal.
    //
    void setRownoHex(bool hex);

    void setEditorFocus(bool focus);

    void setFirstHighlight(int highlight);

    void setSecondHighlight(int highlight);

    int trackFromMouse(QPoint const& pos) const;

protected:

    void changeEvent(QEvent *evt) override;

    virtual void dragEnterEvent(QDragEnterEvent *evt) override;

    virtual void dragLeaveEvent(QDragLeaveEvent *evt) override;

    virtual void dragMoveEvent(QDragMoveEvent *evt) override;

    virtual void dropEvent(QDropEvent *evt) override;

    void leaveEvent(QEvent *evt) override;

    void mouseMoveEvent(QMouseEvent *evt) override;

    void mousePressEvent(QMouseEvent *evt) override;

    void mouseReleaseEvent(QMouseEvent *evt) override;

    void paintEvent(QPaintEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;

private:
    Q_DISABLE_COPY(PatternGrid)
    
    void updateCursorRow();
    void updateAll();
    void setPlaying(bool playing);

    void updateCursor(PatternModel::CursorChangeFlags flags);

    //
    // Called when appearance settings have changed, recalculates metrics and redraws
    // all rows.
    //
    void fontChanged();

    int mouseToRow(int const mouseY);

    PatternCursor mouseToCursor(QPoint const pos);

    bool rowIsValid(int row);

    void clampCursor(PatternCursor &cursor);

    void calculateTrackerRow();

    PatternGridHeader &mHeader;
    PatternModel &mModel;
    PatternLayout mLayout;
    PatternPainter mPainter;

    bool mShowShadow;

    bool mSelecting;

    int mVisibleRows; // number of rows visible on the widget
    // mVisibleRows * mRowHeight is always >= height()

    // cached value of the tracker player position
    // saved here so we don't have to calculate it every paint event
    std::optional<int> mTrackerRow;

    bool mEditorFocus;

    // user must move this amount of pixels to begin selecting
    static constexpr auto SELECTION_DEAD_ZONE = 4;

    enum class MouseOperation {
        nothing,            // do nothing
        selectingRows,      // selecting whole rows
        beginSelecting,     // selecting data
        selecting,
        dragging            // drag n drop
    };

    QPoint mMousePos;
    // grid coordinates of the selection being made by the user
    // if both are set, then the user has made a valid selection
    PatternCursor mSelectionStart;
    PatternCursor mSelectionEnd;

    bool mHasDrag;
    PatternCursor mDragPos;
    // the row, relative to the current selection, in which the user started dragging from
    int mDragRow;

    MouseOperation mMouseOp;



};

