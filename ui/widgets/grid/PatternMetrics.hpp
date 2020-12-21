
#pragma once


struct PatternMetrics {

    constexpr PatternMetrics() :
        offset(0),
        trackWidth(0),
        rownoWidth(0)
    {

    }

    // x offset for centering
    int offset;

    // width, in pixels, of a track
    int trackWidth;
    // width, in pixels, of the row number column
    int rownoWidth;


};
