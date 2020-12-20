
#include "widgets/PatternEditor.hpp"

#include "misc/utils.hpp"

#include <QGridLayout>

PatternEditor::PatternEditor(SongListModel &model, ColorTable const &colorTable, QWidget *parent) :
    QFrame(parent),
    mLayout(),
    mGrid(model, colorTable),
    mHScroll(Qt::Horizontal),
    mVScroll(Qt::Vertical),
    mWheel(0),
    mPageStep(4)
{
    setFrameStyle(QFrame::StyledPanel);
    setFocusPolicy(Qt::StrongFocus);

    mHScroll.setMinimum(0);
    mHScroll.setMaximum(47);
    mHScroll.setPageStep(1);

    mVScroll.setMinimum(0);
    mVScroll.setMaximum(63);
    mVScroll.setPageStep(mPageStep);

    mLayout.setMargin(0);
    mLayout.setSpacing(0);
    mLayout.addWidget(&mGrid, 0, 0);
    mLayout.addWidget(&mVScroll, 0, 1);
    mLayout.addWidget(&mHScroll, 1, 0);
    setLayout(&mLayout);


    setupAction(mActions.undo, "&Undo", "Undos the last operation", Icons::editUndo, QKeySequence::Undo);
    setupAction(mActions.redo, "&Redo", "Redos the last operation", Icons::editRedo, QKeySequence::Redo);
    setupAction(mActions.cut, "C&ut", "Copies and deletes selection to the clipboard", Icons::editCut, QKeySequence::Cut);
    setupAction(mActions.copy, "&Copy", "Copies selected rows to the clipboard", Icons::editCopy, QKeySequence::Copy);
    setupAction(mActions.paste, "&Paste", "Pastes contents at the cursor", Icons::editPaste, QKeySequence::Paste);
    setupAction(mActions.pasteMix, "Paste &Mix", "Pastes contents at the cursor, merging with existing rows", tr("Ctrl+M"));
    setupAction(mActions.delete_, "&Delete", "Deletes selection", QKeySequence::Delete);
    setupAction(mActions.selectAll, "&Select All", "Selects entire track/pattern", QKeySequence::SelectAll);
    setupAction(mActions.noteIncrease, "Increase note", "Increases note/notes by 1 step");
    setupAction(mActions.noteDecrease, "Decrease note", "Decreases note/notes by 1 step");
    setupAction(mActions.octaveIncrease, "Increase octave", "Increases note/notes by 12 steps");
    setupAction(mActions.octaveDecrease, "Decrease octave", "Decreases note/notes by 12 steps");
    setupAction(mActions.reverse, "&Reverse", "Reverses selected rows");

    mTransposeMenu.setTitle(tr("&Transpose"));
    mTransposeMenu.addAction(&mActions.noteIncrease);
    mTransposeMenu.addAction(&mActions.noteDecrease);
    mTransposeMenu.addAction(&mActions.octaveIncrease);
    mTransposeMenu.addAction(&mActions.octaveDecrease);


    connect(&mGrid, &PatternGrid::cursorRowChanged, &mVScroll, &QScrollBar::setValue);
    connect(&mVScroll, &QScrollBar::valueChanged, &mGrid, &PatternGrid::setCursorRow);
    connect(&mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);

    connect(&mGrid, &PatternGrid::cursorColumnChanged, &mHScroll, &QScrollBar::setValue);
    connect(&mHScroll, &QScrollBar::valueChanged, &mGrid, &PatternGrid::setCursorColumn);
    connect(&mHScroll, &QScrollBar::actionTriggered, this, &PatternEditor::hscrollAction);

    connect(&model, &SongListModel::patternSizeChanged, this,
        [this](int rows) {
            mVScroll.setMaximum(rows - 1);
        });

}

PatternEditor::Actions& PatternEditor::menuActions() {
    return mActions;
}

void PatternEditor::setAppearance(Config::Appearance const& appearance) {
    mGrid.setFont(appearance.font);
    mGrid.apply();
}

void PatternEditor::setupMenu(QMenu &menu) {
    menu.addAction(&mActions.undo);
    menu.addAction(&mActions.redo);
    menu.addSeparator();
    menu.addAction(&mActions.cut);
    menu.addAction(&mActions.copy);
    menu.addAction(&mActions.paste);
    menu.addAction(&mActions.pasteMix);
    menu.addAction(&mActions.delete_);
    menu.addSeparator();
    menu.addAction(&mActions.selectAll);
    menu.addSeparator();
    menu.addMenu(&mTransposeMenu);
    menu.addAction(&mActions.reverse);



}

void PatternEditor::keyPressEvent(QKeyEvent *evt) {
    switch (evt->key()) {
        case Qt::Key_Left:
            mGrid.moveCursorColumn(-1);
            break;
        case Qt::Key_Right:
            mGrid.moveCursorColumn(1);
            break;
        case Qt::Key_Up:
            mGrid.moveCursorRow(-1);
            break;
        case Qt::Key_Down:
            mGrid.moveCursorRow(1);
            break;
        case Qt::Key_PageDown:
            mGrid.moveCursorRow(mPageStep);
            break;
        case Qt::Key_PageUp:
            mGrid.moveCursorRow(-mPageStep);
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
        mGrid.moveCursorRow(amount);
    }

    evt->accept();
}

void PatternEditor::hscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mGrid.moveCursorColumn(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mGrid.moveCursorColumn(-1);
            break;
        default:
            break;
    }
}

void PatternEditor::vscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mGrid.moveCursorRow(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mGrid.moveCursorRow(-1);
            break;
        default:
            break;
    }
}
