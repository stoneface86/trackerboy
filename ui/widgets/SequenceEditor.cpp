
#include "widgets/SequenceEditor.hpp"

#include <QPainter>
#include <QEvent>

constexpr int PADDING_Y = 4;
constexpr int LOOP_ARROW_HEIGHT = 10;

/*

+-----------------------+
|  _____________________|
|  |                    |
|  |  GRAPH EDIT        | VScroll (in SequenceEditor)
|  |                    |
|  |                    |
|  |____________________|
+-----------------------+
 ^
 |
 Y axis (min and maximum shown)

        +--+------------------------+--+
        +  | Loop select            |  |
        +--+------------------------+--+
        |  |                        |  |
        |  |                        |  |
 Axis ---> | Graph editor           | <---- VScrollbar
        |  |                        |  |
        |  |                        |  |
        |--+------------------------+--+
        |  | HScrollbar             |  |
        +--+------------------------+--+



*/




SequenceEditor::SequenceEditor(QWidget *parent) :
    QWidget(parent),
    mLayout(),
    mAxisLayout(),
    mMaxLabel(),
    mMinLabel(),
    mVScroll(Qt::Vertical),
    mHScroll(Qt::Horizontal),
    mData(),
    mCellWidth(0),
    mCellHeight(0)
{
    mAxisLayout.addWidget(&mMaxLabel);
    mAxisLayout.addStretch(1);
    mAxisLayout.addWidget(&mMinLabel);

    //mLayout.addWidget(&mLoopSelect, 0, 1);
    mLayout.addLayout(&mAxisLayout, 1, 0);
    //mLayout.addWidget(&mGraph, 1, 1);
    mLayout.addWidget(&mVScroll, 1, 2);
    mLayout.addWidget(&mHScroll, 2, 1);
    mLayout.setColumnStretch(1, 1);
    setLayout(&mLayout);

}


