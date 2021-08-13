
#pragma once

#include "core/PatternCursor.hpp"
#include "core/PatternSelection.hpp"

#include <QPoint>
#include <QRect>

#include <array>

//
// Utility class used by PatternPainter for determining the layout of pattern
// data to be painted. Also contains methods for converting mouse positions
// to/from pattern cursors.
//
class PatternLayout {

public:

    //
    // amount of pixels between select columns
    //
    static constexpr int SPACING = 4;

    //
    // width, in pixels, of the lines to be drawn between tracks
    //
    static constexpr int LINE_WIDTH = 1;

    //
    // Default constructor. Must call setCellSize before using this layout!
    //
    PatternLayout();

    //
    // Sets the cell dimensions and updates layout geometry.
    //
    void setCellSize(int width, int height);

    //
    // X-position of where to start painting pattern data.
    //
    int patternStart() const;

    //
    // Width of an entire pattern row.
    //
    int rowWidth() const;

    //
    // Gets the width for the given track.
    //
    int trackWidth(int track) const;

    //
    // Gets the x-position of the given track.
    //
    int trackToX(int track) const;

    //
    // Gets the x-position relative to a track for a given column. To get the
    // absolute position, add the result of this function to the result of trackToX
    // for the column's track.
    //
    int columnToX(int column) const;

    //
    // Converts a coordinate to a pattern cursor. May not return a valid
    // cursor, so check for validity or clamp afterwards.
    //
    PatternCursor mouseToCursor(QPoint point) const;

    int mouseToTrack(int x) const;

    //
    // Converts the pattern selection to a rectangle for painting. 
    //
    QRect selectionRectangle(PatternSelection const& selection) const;

private:

    int mCellWidth;     // width of a cell/char
    int mCellHeight;    // height of a cell/char

    int mPatternStart;  // starting x-position to draw patterns
    int mTrackWidth;    // width of a single track

    int mPatternWidth;

    // column dividers, used for converting a mouse coordinate to column
    // example, | is a divider (NNN - note, I - instrument cell, E - effect cell)
    // NNN | I|I | E|E|E | E|E|E | E|E|E
    std::array<int, PatternCursor::MAX_COLUMNS - 1> mColumnDivs;


};
