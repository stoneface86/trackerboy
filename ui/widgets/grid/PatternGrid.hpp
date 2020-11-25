
#pragma once

#include "model/OrderModel.hpp"

#include "widgets/grid/PatternGridHeader.hpp"

#include <QWidget>
#include <QPaintEvent>
#include <QString>

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

namespace grid {

class PatternGrid : public QWidget {

    Q_OBJECT

public:

    // background colors
    static constexpr int COLOR_BG = 0;
    static constexpr int COLOR_BG_HIGHLIGHT = 1;
    static constexpr int COLOR_BG_ROW = 2;

    // foreground colors (text)

    // notes / row number
    static constexpr int COLOR_FG = 3;
    // highlighted row
    static constexpr int COLOR_FG_HIGHLIGHT = 4;
    // instrument column
    static constexpr int COLOR_FG_INSTRUMENT = 5;
    // effect column
    static constexpr int COLOR_FG_EFFECT_TYPE = 6;
    // effect argument columns
    static constexpr int COLOR_FG_EFFECT_ARG = 7;

    // selection rectangle
    static constexpr int COLOR_SELECTION = 8;
    // cursor rectangle
    static constexpr int COLOR_CURSOR = 9;
    // lines between tracks
    static constexpr int COLOR_LINE = 10;


    static constexpr int COLOR_COUNT = 11;

    explicit PatternGrid(OrderModel &model, QWidget *parent = nullptr);
    ~PatternGrid() = default;

    // Settings

    //void apply(); // applies current settings

    //
    // Sets the number of visible effect columns for the given track (1-3)
    //
    void setEffectColumns(unsigned track, unsigned columns);

    int row() const;

signals:

    // emitted when the user changes the current pattern
    void cursorPatternChanged(int patternNo);

    // emitted when the user selects a different track via keyboard or mouse
    void cursorTrackChanged(int track);

    void cursorColumnChanged(int column);

    // emitted when the user changes the current row via keyboard, scroll wheel or mouse
    void cursorRowChanged(int row);

    // emitted when the header needs to be redrawn
    void updateHeader();

public slots:

    void setCursorTrack(int track);
    void setCursorRow(int row);

    void cursorLeft();
    void cursorRight();
    void cursorUp();
    void cursorDown();


protected:

    virtual void mouseMoveEvent(QMouseEvent *evt) override;

    virtual void mousePressEvent(QMouseEvent *evt) override;

    virtual void mouseReleaseEvent(QMouseEvent *evt) override;

    virtual void keyPressEvent(QKeyEvent *evt) override;

    virtual void paintEvent(QPaintEvent *evt) override;

    virtual void resizeEvent(QResizeEvent *evt) override;

    void wheelEvent(QWheelEvent *evt) override;

    //virtual void timerEvent(QTimerEvent *evt) override;

private:

    //
    // Called when appearance settings have changed, recalculates metrics and redraws
    // all rows.
    //
    void appearanceChanged();

    //
    // Calculates the number of rows we can draw on this widget
    //
    unsigned getVisibleRows();

    void getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol);

    //
    // Paints rows using the given painter. Row indices range form 0 to mVisibleRows.
    // Row 0 is the first visible row on the widget.
    // 0 <= rowStart < rowEnd <= mVisibleRows
    //
    void paintRows(QPainter &painter, int rowStart, int rowEnd);

    //
    // Scrolls displayed rows and paints new ones. If rows >= mVisibleRows then
    // no scrolling will occur and the entire display will be repainted
    //
    void scroll(int rows);

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

    struct Column {
        uint8_t track;
        ColumnType type;
        uint8_t location;
    };

    OrderModel &mModel;

    // display image, rows get painted here when needed as opposed to every
    // paintEvent
    QPixmap mDisplay;

    std::array<QColor, COLOR_COUNT> mColorTable;

    std::array<unsigned, 4> mEffectsVisible; // number of effects visible for each track
    std::array<unsigned, 6> mLineCells;

    // cell lookup vector
    // given a character index, we can determine which track and column the cell
    // belongs to, only needed for converting mouse coordinates to cell coordinates
    std::vector<uint8_t> mCellLayout;

    // column layout, maps a column index -> Column
    std::vector<Column> mColumns;



    // if true all rows will be redrawn on next paint event
    bool mRepaintImage;

    unsigned mCursorRow;
    unsigned mCursorCol;
    unsigned mCursorPattern;    // the current pattern

    unsigned mPatterns;
    unsigned mPatternSize;

    unsigned mPageStep;
    int mWheel;

    bool mSelecting;

    // translation x coordinate for centering the grid
    int mDisplayXpos;

    // variables here are dependent on appearance settings

    // metrics
    unsigned mRowHeight; // height of a row in pixels, including padding
    //unsigned mRowHeightHalf;
    //unsigned mCenter;
    unsigned mCharWidth; // width of a character
    unsigned mVisibleRows; // number of rows visible on the widget
    // mVisibleRows * mRowHeight is always >= height()


};

}
