
#include "widgets/PatternEditor.hpp"

#include <QGridLayout>

PatternEditor::PatternEditor(SongListModel &model, QWidget *parent) :
    QFrame(parent),
    mGrid(new PatternGrid(model, this)),
    mHScroll(new QScrollBar(Qt::Horizontal, this)),
    mVScroll(new QScrollBar(Qt::Vertical, this)),
    mWheel(0),
    mPageStep(4)
{
    setFrameStyle(QFrame::StyledPanel);
    setFocusPolicy(Qt::StrongFocus);

    mHScroll->setMinimum(0);
    mHScroll->setMaximum(47);
    mHScroll->setPageStep(1);

    mVScroll->setMinimum(0);
    mVScroll->setMaximum(63);
    mVScroll->setPageStep(mPageStep);

    QGridLayout *layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mGrid, 0, 0);
    layout->addWidget(mVScroll, 0, 1);
    layout->addWidget(mHScroll, 1, 0);
    setLayout(layout);

    connect(mGrid, &PatternGrid::cursorRowChanged, mVScroll, &QScrollBar::setValue);
    connect(mVScroll, &QScrollBar::valueChanged, mGrid, &PatternGrid::setCursorRow);
    connect(mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);

    connect(mGrid, &PatternGrid::cursorColumnChanged, mHScroll, &QScrollBar::setValue);
    connect(mHScroll, &QScrollBar::valueChanged, mGrid, &PatternGrid::setCursorColumn);
    connect(mHScroll, &QScrollBar::actionTriggered, this, &PatternEditor::hscrollAction);

    connect(&model, &SongListModel::patternSizeChanged, this,
        [this](int rows) {
            mVScroll->setMaximum(rows - 1);
        });

}

void PatternEditor::keyPressEvent(QKeyEvent *evt) {
    switch (evt->key()) {
        case Qt::Key_Left:
            mGrid->moveCursorColumn(-1);
            break;
        case Qt::Key_Right:
            mGrid->moveCursorColumn(1);
            break;
        case Qt::Key_Up:
            mGrid->moveCursorRow(-1);
            break;
        case Qt::Key_Down:
            mGrid->moveCursorRow(1);
            break;
        case Qt::Key_PageDown:
            mGrid->moveCursorRow(mPageStep);
            break;
        case Qt::Key_PageUp:
            mGrid->moveCursorRow(-mPageStep);
            break;
        default:
            QWidget::keyPressEvent(evt);
            break;
    }
}

void PatternEditor::wheelEvent(QWheelEvent *evt) {
    mWheel += evt->angleDelta().y();
    int amount = 0;
    // 120 / 8 = 15 degrees
    if (mWheel >= 120) {
        mWheel -= 120;
        amount = -mPageStep;
    } else if (mWheel <= -120) {
        mWheel += 120;
        amount = mPageStep;
    }

    if (amount) {
        mGrid->moveCursorRow(amount);
    }

    evt->accept();
}

void PatternEditor::hscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mGrid->moveCursorColumn(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mGrid->moveCursorColumn(-1);
            break;
        default:
            break;
    }
}

void PatternEditor::vscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mGrid->moveCursorRow(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mGrid->moveCursorRow(-1);
            break;
        default:
            break;
    }
}
