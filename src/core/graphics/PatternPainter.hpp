
#pragma once

#include "core/graphics/CachedPen.hpp"
#include "core/graphics/CellPainter.hpp"
#include "core/graphics/PatternLayout.hpp"
#include "core/Palette.hpp"
#include "core/PatternCursor.hpp"
#include "core/PatternSelection.hpp"
#include "core/misc/NoteStrings.hpp"

#include "trackerboy/data/Pattern.hpp"

#include <QColor>

#include <array>


class PatternPainter : public CellPainter {

public:

    enum RowType {
        RowCurrent,
        RowEdit,
        RowPlayer
    };

    PatternPainter(QFont const& font);

    //
    // Returns true if accidentals will be drawn using flats instead of sharps
    //
    bool flats() const;

    void setColors(Palette const& colors);

    void setFirstHighlight(int interval);

    void setSecondHighlight(int interval);

    void setFlats(bool flats);

    // drawing functions

    //
    // draws the background for pattern data.
    //
    void drawBackground(QPainter &p, PatternLayout const& l, int ypos, int rowStart, int rows) const;


    //
    // Draws the cursor row background or player row background
    //
    void drawRowBackground(QPainter &p, PatternLayout const& l, RowType type, int row) const;

    void drawCursor(QPainter &p, PatternLayout const& l, PatternCursor cursor) const;

    void drawLines(QPainter &p, PatternLayout const& l, int height) const;

    //
    // Draws pattern data from the given pattern and range of rows starting
    // at the given y position. The y position of the next row is returned
    //
    int drawPattern(
        QPainter &p, PatternLayout const& l,
        trackerboy::Pattern const& pattern,
        int rowStart,
        int rowEnd,
        int ypos
    ) const;

    //
    // Draws the selection rectangle
    //
    void drawSelection(QPainter &painter, QRect const& rect) const;

    //
    // Paints "nothing" or "-" for the cell(s). The x position of the next
    // cell is returned
    //
    int drawNone(QPainter &painter, int cells, int xpos, int ypos) const;

    //
    // Paints a note at the given x and y position. The x position of the next
    // cell is returned.
    //
    int drawNote(QPainter &painter, uint8_t note, int xpos, int ypos) const;

private:

    int highlightIndex(int rowno) const;
    
    int mHighlightInterval1;
    int mHighlightInterval2;

    NoteStrings::NoteTable const *mNoteTable;

    std::array<QColor, 3> mForegroundColors;
    std::array<QColor, 3> mBackgroundColors;

    QColor mColorInstrument;
    QColor mColorEffect;

    QColor mColorSelection;
    QColor mColorCursor;
    QColor mColorLine;

    std::array<QColor, 3> mRowColors;

    CachedPen mutable mPen;


};
