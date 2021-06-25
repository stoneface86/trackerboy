

#pragma once


//
// Same as PatternCursor, but the column field is a select column. Two anchors
// are used to make a PatternSelection
//
struct PatternAnchor {

    //
    // Selectable columns
    //
    enum SelectType {
        SelectNote,
        SelectInstrument,
        SelectEffect1,
        SelectEffect2,
        SelectEffect3
    };

    static constexpr int MAX_SELECTS = SelectEffect3 + 1;


    int row;
    int select;
    int track;

};
