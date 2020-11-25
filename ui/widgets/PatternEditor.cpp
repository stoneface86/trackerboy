
#include "widgets/PatternEditor.hpp"

#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/PatternGridHeader.hpp"

using namespace grid;

#include <QGridLayout>

PatternEditor::PatternEditor(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mGrid(new PatternGrid(model, this)),
    mGridHeader(new PatternGridHeader(this)),
    mHScroll(new QScrollBar(Qt::Horizontal, this)),
    mVScroll(new QScrollBar(Qt::Vertical, this))
{
    mHScroll->setMinimum(0);
    //mHScroll->setMaximum(mGrid->cells());
    mHScroll->setPageStep(1);

    mVScroll->setMinimum(0);
    mVScroll->setMaximum(63);
    mVScroll->setPageStep(1);

    QGridLayout *layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mGridHeader, 0, 0);
    layout->addWidget(mGrid, 1, 0);
    layout->addWidget(mVScroll, 0, 1, 2, 1);
    layout->addWidget(mHScroll, 2, 0);
    setLayout(layout);

    //connect(mVScroll, &QScrollBar::valueChanged, mGrid, &PatternGrid::setCursorRow);
    connect(mGrid, &PatternGrid::cursorRowChanged, mVScroll, &QScrollBar::setValue);
    connect(mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);

    //connect(mGrid, &PatternGrid::pageSizeChanged, mVScroll, &QScrollBar::setMaximum);
    //connect(mGrid, &PatternGrid::cursorRowChanged, mVScroll, &QScrollBar::setValue);

    //connect(mGrid, &PatternGrid::cellCountChanged, mHScroll, &QScrollBar::setMaximum);
    //connect(mGrid, &PatternGrid::cursorColumnChanged, mVScroll, &QScrollBar::setValue);

}

void PatternEditor::vscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
        case QAbstractSlider::SliderPageStepAdd:
            mGrid->cursorDown();
            break;
        case QAbstractSlider::SliderSingleStepSub:
        case QAbstractSlider::SliderPageStepSub:
            mGrid->cursorUp();
            break;
        case QAbstractSlider::SliderToMinimum:
        case QAbstractSlider::SliderToMaximum:
        case QAbstractSlider::SliderMove:
            mGrid->setCursorRow(mVScroll->sliderPosition());
            break;
        default:
            break;


    }
}
