
#include "widgets/PatternEditor.hpp"

#include "misc/utils.hpp"
#include "widgets/grid/layout.hpp"

#include <QGridLayout>
#include <QtDebug>



// PatternEditor is just a composite widget containing the grid, header and
// two scrollbars. (PatternGrid does most of the work)
//
// +-------------------------------------+
// | PatternGridHeader                   |
// |                                     |
// +----------------------------------+--+
// |                                  |  |
// |                                  |  |
// |                                  |  |
// |                                  |  |
// | PatternGrid                      |  | <--- VScrollBar
// |                                  |  |
// |                                  |  |
// |                                  |  |
// |                                  |  |
// +----------------------------------+--+
// | HScrollBar                       |XX|
// +----------------------------------+--+



PatternEditor::PatternEditor(PianoInput const& input, ModuleDocument &document, QWidget *parent) :
    QFrame(parent),
    mPianoIn(input),
    mLayout(),
    mControls(),
    mControlsLayout(),
    mToolbar(),
    mOctaveLabel(tr("Octave")),
    mOctaveSpin(),
    mEditStepLabel(tr("Edit step")),
    mEditStepSpin(),
    mLoopPatternCheck(tr("Loop pattern")),
    mFollowModeCheck(tr("Follow-mode")),
    mKeyRepeatCheck(tr("Key repetition")),
    mEditorLayout(),
    mOrderWidget(document.orderModel()),
    mGridFrame(),
    mGridLayout(),
    mGridHeader(),
    mGrid(document, mGridHeader),
    mHScroll(Qt::Horizontal),
    mVScroll(Qt::Vertical),
    mWheel(0),
    mPageStep(4)
{

    //setFrameStyle(QFrame::Panel | QFrame::Raised);
    setFocusPolicy(Qt::StrongFocus);

    mHScroll.setMinimum(0);
    mHScroll.setMaximum(47);
    mHScroll.setPageStep(1);

    mVScroll.setMinimum(0);
    mVScroll.setMaximum(63);
    mVScroll.setPageStep(mPageStep);

    mControlsLayout.addWidget(&mToolbar);
    mControlsLayout.addWidget(&mOctaveLabel);
    mControlsLayout.addWidget(&mOctaveSpin);
    mControlsLayout.addWidget(&mEditStepLabel);
    mControlsLayout.addWidget(&mEditStepSpin);
    mControlsLayout.addWidget(&mLoopPatternCheck);
    mControlsLayout.addWidget(&mFollowModeCheck);
    mControlsLayout.addWidget(&mKeyRepeatCheck);
    mControlsLayout.addStretch(1);
    mControls.setLayout(&mControlsLayout);

    mGridLayout.setMargin(0);
    mGridLayout.setSpacing(0);
    mGridLayout.addWidget(&mGridHeader,     0, 0);
    mGridLayout.addWidget(&mGrid,           1, 0);
    mGridLayout.addWidget(&mVScroll,        0, 1, 2, 1);
    mGridLayout.addWidget(&mHScroll,        2, 0);
    mGridFrame.setLayout(&mGridLayout);
    mGridFrame.setFrameStyle(QFrame::StyledPanel);

    mEditorLayout.addWidget(&mOrderWidget);
    mEditorLayout.addWidget(&mGridFrame, 1);

    mLayout.setMargin(0);
    mLayout.setSpacing(0);
    mLayout.addWidget(&mControls);
    mLayout.addLayout(&mEditorLayout, 1);
    setLayout(&mLayout);

    mEditStepSpin.setRange(0, 256);
    mOctaveSpin.setRange(2, 8);

    setupAction(mTrackerActions.play, "Play at cursor", "Play from the cursor", Icons::patternPlay);
    setupAction(mTrackerActions.restart, "Play pattern", "Plays at the start of the current pattern", Icons::patternRestart);
    setupAction(mTrackerActions.playRow, "Play row", "Plays the current row", Icons::patternPlayRow);
    setupAction(mTrackerActions.record, "Record", "Toggles record mode", Icons::patternRecord);

    mToolbar.setIconSize(QSize(16, 16));
    mToolbar.addAction(&mTrackerActions.play);
    mToolbar.addAction(&mTrackerActions.restart);
    mToolbar.addAction(&mTrackerActions.playRow);
    mToolbar.addAction(&mTrackerActions.record);
    mTrackerActions.record.setCheckable(true);


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

    connect(&mTrackerActions.record, &QAction::toggled, &mGrid, &PatternGrid::setRecord);

    connect(&mGrid, &PatternGrid::cursorRowChanged, &mVScroll, &QScrollBar::setValue);
    connect(&mVScroll, &QScrollBar::valueChanged, &mGrid, &PatternGrid::setCursorRow);
    connect(&mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);

    connect(&mGrid, &PatternGrid::cursorColumnChanged, &mHScroll, &QScrollBar::setValue);
    connect(&mHScroll, &QScrollBar::valueChanged, &mGrid, &PatternGrid::setCursorColumn);
    connect(&mHScroll, &QScrollBar::actionTriggered, this, &PatternEditor::hscrollAction);

    //connect(&model, &SongListModel::patternSizeChanged, this,
    //    [this](int rows) {
    //        mVScroll.setMaximum(rows - 1);
    //    });

    connect(&mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::octaveChanged);

    mOctaveSpin.setValue(4);
    mFollowModeCheck.setCheckState(Qt::Checked);
    mKeyRepeatCheck.setCheckState(Qt::Checked);

    connect(&mFollowModeCheck, &QCheckBox::stateChanged, &mGrid, &PatternGrid::setFollowMode);
}

PatternGrid& PatternEditor::grid() {
    return mGrid;
}

PatternEditor::Actions& PatternEditor::menuActions() {
    return mActions;
}

PatternEditor::TrackerActions& PatternEditor::trackerActions() {
    return mTrackerActions;
}

void PatternEditor::setupMenu(QMenu &menu) {
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

void PatternEditor::setColors(ColorTable const& colors) {
    mGridHeader.setColors(colors);
    mGrid.setColors(colors);
}

void PatternEditor::keyPressEvent(QKeyEvent *evt) {
    
    
    int const key = evt->key();
    
    
    // navigation keys / non-edit keys
    // these keys also ignore the key repetition setting (they always repeat)
    switch (key) {
        case Qt::Key_Left:
            mGrid.moveCursorColumn(-1);
            return;
        case Qt::Key_Right:
            mGrid.moveCursorColumn(1);
            return;
        case Qt::Key_Up:
            mGrid.moveCursorRow(-1);
            return;
        case Qt::Key_Down:
            mGrid.moveCursorRow(1);
            return;
        case Qt::Key_PageDown:
            mGrid.moveCursorRow(mPageStep);
            return;
        case Qt::Key_PageUp:
            mGrid.moveCursorRow(-mPageStep);
            return;
        case Qt::Key_Space:
            mTrackerActions.record.toggle();
            return;
    }

    if (evt->isAutoRepeat() && !mKeyRepeatCheck.isChecked()) {
        QWidget::keyPressEvent(evt);
        return; // key repetition disabled, ignore this event
    }

    if (mGrid.processKeyPress(mPianoIn, key)) {
        mGrid.moveCursorRow(mEditStepSpin.value());
        
    } else {
        // invalid key or edit mode is off, let QWidget handle it
        QWidget::keyPressEvent(evt);
    }
    
}

void PatternEditor::keyReleaseEvent(QKeyEvent *evt) {
    if (evt->key() == mPreviewKey) {
        // TODO: stop preview
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
