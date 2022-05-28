
#pragma once

#include "graphics/CachedPen.hpp"
#include "graphics/CellPainter.hpp"
#include "model/PatternModel.hpp"
#include "config/data/Palette.hpp"
#include "verdigris/wobjectdefs.h"

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QWidget>

#include <optional>

class OrderGrid : public QWidget {

    W_OBJECT(OrderGrid)

public:

    explicit OrderGrid(PatternModel &model, QWidget *parent = nullptr);

    void setColors(Palette const& colors);

    void setChangeAll(bool changeAll);

    void decrement();

    void increment();

//signals:
    void patternJump(int pattern) W_SIGNAL(patternJump, pattern)

protected:

    virtual void changeEvent(QEvent *evt) override;

    virtual void keyPressEvent(QKeyEvent *evt) override;

    virtual void mousePressEvent(QMouseEvent *evt) override;

    virtual void mouseMoveEvent(QMouseEvent *evt) override;

    virtual void mouseReleaseEvent(QMouseEvent *evt) override;

    virtual void paintEvent(QPaintEvent *evt) override;

    virtual void resizeEvent(QResizeEvent *evt) override;

private:
    Q_DISABLE_COPY(OrderGrid)

    void fontChanged();

    int rowsVisisble() const;

    int cursorX() const;

    void incDec(int amount);

    //
    // Recalculate the range of rows to draw on this widget. Call this when
    //  - the cursor changes
    //  - the number of patterns changes
    //  - the height of the widget changes
    //
    void updatePatternRange();

    void setCursorPattern(int pattern);

    // spacing, in pixels, for the row number column
    static constexpr int SPACING = 4;
    // width, in pixels, of the line following the row number column
    static constexpr int LINE_WIDTH = 1;

    PatternModel &mModel;

    CellPainter mCellPainter;
    QColor mLineColor;
    QColor mRownoColor;
    QColor mTextColor;
    QColor mRowColor;
    QColor mTrackerColor;
    QColor mCursorColor;

    CachedPen mPen;

    QRect mGridRect;    // boundary rect of the grid
    std::optional<QPoint> mMousePressedPos;

    int mVisibleRows;   // number of rows we can draw (the last row may be clipped)
    bool mHighNibble;   // true if cursor is over the high nibble of the id
    bool mChangeAll;    // true if we are editing all tracks

    // range of the order to draw
    int mPatternStart;
    int mPatternEnd;

};
