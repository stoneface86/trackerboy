
#pragma once

#include "core/graphics/CellPainter.hpp"
#include "core/Palette.hpp"
#include "core/PatternCursor.hpp"
#include "core/PatternSelection.hpp"

#include "trackerboy/data/PatternRow.hpp"

#include <QColor>
#include <QPen>
#include <QFont>
#include <QPainter>
#include <QWidget>

#include <array>


class PatternPainter : public CellPainter {

public:

    enum RowType {
        ROW_CURRENT,
        ROW_EDIT,
        ROW_PLAYER
    };

    PatternPainter(QFont const& font);

    int rownoWidth() const;

    int trackWidth() const;

    //
    // Returns true if accidentals will be drawn using flats instead of sharps
    //
    bool flats() const;

    QRect selectionRectangle(PatternSelection const& selection);

    void setColors(Palette const& colors);

    void setFirstHighlight(int interval);

    void setSecondHighlight(int interval);

    void setFlats(bool flats);

    // drawing functions

    void drawBackground(QPainter &painter, int ypos, int rowStart, int rows);

    void drawRowBackground(QPainter &painter, RowType type, int row);

    void drawCursor(QPainter &painter, PatternCursor cursor);

    void drawLines(QPainter &painter, int height);

    //
    // Paints the full row, including the row number, at the given y-position. Returns the
    // y-position of the next row
    //
    int drawRow(QPainter &textPainter,
                trackerboy::PatternRow const& rowdata, 
                int rowno, 
                int ypos
                );
    
    //
    // Draws the selection rectangle
    //
    void drawSelection(QPainter &painter, PatternSelection const& selection);

    //
    // Paints "nothing" or "-" for the cell(s)
    //
    void drawNone(QPainter &painter, int cells, int xpos, int ypos);

    //
    // Paints a note at the given x and y position
    //
    void drawNote(QPainter &painter, uint8_t note, int xpos, int ypos);

protected:

    virtual void cellSizeChanged(int width, int height) override;

private:

    using NoteTable = std::array<char, 24>;

    static NoteTable const NOTE_TABLE_FLATS;
    static NoteTable const NOTE_TABLE_SHARPS;

    //
    // Returns a reference to the cached QPen after setting its color to the
    // given parameter. Use this function instead of creating a temporary QPen
    //
    QPen const& pen(QColor const& color);

    int highlightIndex(int rowno);
    
    int mHighlightInterval1;
    int mHighlightInterval2;

    int mRownoWidth;
    int mTrackWidth;
    int mPatternWidth;

    NoteTable const *mNoteTable;

    std::array<QColor, 3> mForegroundColors;
    std::array<QColor, 3> mBackgroundColors;


    QColor mColorInstrument;
    QColor mColorEffect;

    QColor mColorSelection;
    QColor mColorCursor;
    QColor mColorLine;

    std::array<QColor, 3> mRowColors;

    // pen used in all draw functions, reusing this one prevents the
    // need to create a temporary
    QPen mPen;


};
