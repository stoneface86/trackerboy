
#pragma once

#include "core/ColorTable.hpp"

#include "trackerboy/data/PatternRow.hpp"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QWidget>

#include <array>


class PatternPainter {

public:

    enum RowType {
        ROW_CURRENT,
        ROW_EDIT,
        ROW_PLAYER
    };

    PatternPainter(QFont const& font);

    int cellHeight() const;

    int cellWidth() const;

    int rownoWidth() const;

    int trackWidth() const;

    bool flats() const;

    // --

    int columnLocation(int column) const;

    // --

    void setColors(ColorTable const& colors);

    void setFont(QFont const& font);

    void setFirstHighlight(int interval);

    void setSecondHighlight(int interval);

    void setFlats(bool flats);

    // drawing functions

    //void drawBackground(QPainter &painter, int rowStart, int rows);

    void drawRowBackground(QPainter &painter, RowType type, int row);

    void drawCursor(QPainter &painter, int row, int column);

    void drawLines(QPainter &painter, int height);

    //void drawSelection(QPainter &painter, int xpos, int ypos, )

    //
    // Paints the full row, including the row number, at the given y-position. Returns the
    // y-position of the next row
    //
    int drawRow(QPainter &textPainter,
                //QPainter &bgPainter,
                trackerboy::PatternRow const& rowdata, 
                int rowno, 
                int ypos
                );

    //
    // Paints "nothing" or "." for the cell(s)
    //
    void drawNone(QPainter &painter, int cells, int xpos, int ypos);

    //
    // Paints a note at the given x and y position
    //
    void drawNote(QPainter &painter, uint8_t note, int xpos, int ypos);


    // low-level draw function. Draws a single character cell at the given x and y position
    void drawCell(QPainter &painter, char cell, int xpos, int ypos);

    // use when editing
    void drawColumn(QPainter &painter, trackerboy::PatternRow const& data, int col, int ypos);

    void eraseColumn(QPainter &painter, int column, int ypos);

private:

    int highlightIndex(int rowno);
    
    int mHighlightInterval1;
    int mHighlightInterval2;

    int mCellWidth;
    int mCellHeight;

    int mRownoWidth;
    int mTrackWidth;
    int mPatternWidth;

    bool mDisplayFlats;


    std::array<QColor, 3> mForegroundColors;
    std::array<QColor, 3> mBackgroundColors;


    QColor mColorInstrument;
    QColor mColorEffect;

    QColor mColorSelection;
    QColor mColorCursor;
    QColor mColorLine;

    std::array<QColor, 3> mRowColors;


    
    
    
    


    

};
