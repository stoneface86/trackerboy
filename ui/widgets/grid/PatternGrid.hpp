
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "core/ColorTable.hpp"
#include "core/PianoInput.hpp"
#include "widgets/grid/PatternGridHeader.hpp"
#include "widgets/grid/PatternPainter.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/note.hpp"

#include <QWidget>
#include <QPaintEvent>
#include <QString>
#include <QBitmap>
#include <QRect>
#include <QSize>

#include <cstdint>
#include <optional>


class PatternGrid : public QWidget {

    Q_OBJECT

public:

    explicit PatternGrid(PatternGridHeader &header, QWidget *parent = nullptr);
    ~PatternGrid() = default;

    // Settings

    //
    // set color scheme, grid is redrawn afterwards
    //
    void setColors(ColorTable const& colors);

    // show pattern data for the previous and next patterns
    //void setPreviewEnable(bool previews);

    // if true then flats will be shown instead of sharps
    void setShowFlats(bool showFlats);

    void setDocument(ModuleDocument *document);

    void setEditorFocus(bool focus);

protected:

    void changeEvent(QEvent *evt) override;

    void leaveEvent(QEvent *evt) override;

    void mouseMoveEvent(QMouseEvent *evt) override;

    void mousePressEvent(QMouseEvent *evt) override;

    void mouseReleaseEvent(QMouseEvent *evt) override;

    void paintEvent(QPaintEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;

private slots:
    void updateCursorRow();
    void updateAll();
    void setPlaying(bool playing);

    void setFirstHighlight(int highlight);
    void setSecondHighlight(int highlight);

private:
    Q_DISABLE_COPY(PatternGrid)

    //
    // Called when appearance settings have changed, recalculates metrics and redraws
    // all rows.
    //
    void fontChanged();

    //
    // Calculates the number of rows we can draw on this widget
    //
    unsigned getVisibleRows();

    int mouseToRow(int const mouseY);

    int mouseToCell(int const mouseX);

    int mouseToColumn(int const mouseX);

    bool rowIsValid(int row);

    QPoint coordsClamped(QPoint const coords);

    void calculateTrackerRow();

    ModuleDocument *mDocument;
    PatternGridHeader &mHeader;
    PatternPainter mPainter;

    bool mSelecting;

    unsigned mVisibleRows; // number of rows visible on the widget
    // mVisibleRows * mRowHeight is always >= height()

    // cached value of the tracker player position
    // saved here so we don't have to calculate it every paint event
    std::optional<int> mTrackerRow;

    bool mEditorFocus;

    // user must move this amount of pixels to begin selecting
    static constexpr auto SELECTION_DEAD_ZONE = 4;

    bool mMouseLeftDown;

    QPoint mSelectionStartMouse;
    // grid coordinates of the selection being made by the user
    // if both are set, then the user has made a valid selection
    std::optional<QPoint> mSelectionStartCoords;
    std::optional<QPoint> mSelectionEndCoords;
    bool mSelectingRows;

};

