
#pragma once


namespace grid {


enum ColorIndex {

    // background colors
    COLOR_BG,
    COLOR_BG_HIGHLIGHT,
    COLOR_BG_ROW,

    // foreground colors (text)

    // notes / row number
    COLOR_FG,

    // highlighted row
    COLOR_FG_HIGHLIGHT,

    // instrument column
    COLOR_FG_INSTRUMENT,

    // effect column
    COLOR_FG_EFFECT_TYPE,

    // effect argument columns
    COLOR_FG_EFFECT_ARG,

    // selection rectangle
    COLOR_SELECTION,

    // cursor rectangle
    COLOR_CURSOR,

    // lines between tracks
    COLOR_LINE,

    // total colors
    COLOR_COUNT


};

}
