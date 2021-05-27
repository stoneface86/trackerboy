
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



PatternEditor::PatternEditor(PianoInput const& input, QWidget *parent) :
    QFrame(parent),
    mPianoIn(input),
    mDocument(nullptr),
    mLayout(),
    mControls(),
    mControlsLayout(),
    mToolbarLayout(),
    mToolbar(),
    mOctaveLabel(tr("Octave")),
    mOctaveSpin(),
    mEditStepLabel(tr("Edit step")),
    mEditStepSpin(),
    mLoopPatternCheck(tr("Loop pattern")),
    mFollowModeCheck(tr("Follow-mode")),
    mKeyRepeatCheck(tr("Key repetition")),
    mSettingsLayout(),
    mRowsPerBeatLabel(tr("Rows per beat:")),
    mRowsPerBeatSpin(),
    mSpeedLabel(tr("Speed:")),
    mSpeedSpin(),
    mSpeedActualLabel(),
    mPatternsLabel(tr("Patterns:")),
    mPatternsSpin(),
    mInstrumentCombo(),
    mRowsPerMeasureLabel(tr("Rows per measure:")),
    mRowsPerMeasureSpin(),
    mTempoLabel(tr("Tempo:")),
    mTempoSpin(),
    mTempoActualLabel(),
    mPatternSizeLabel(tr("Pattern size:")),
    mPatternSizeSpin(),
    mSetInstrumentCheck(tr("Set instrument column")),
    mLines(),
    mGridFrame(),
    mGridLayout(),
    mGridHeader(),
    mGrid(mGridHeader),
    mHScroll(Qt::Horizontal),
    mVScroll(Qt::Vertical),
    mWheel(0),
    mPageStep(4),
    mSpeedLock(false)
{

    //setFrameStyle(QFrame::Panel | QFrame::Raised);
    setFocusPolicy(Qt::StrongFocus);

    mHScroll.setMinimum(0);
    mHScroll.setMaximum(47);
    mHScroll.setPageStep(1);

    mToolbarLayout.addWidget(&mToolbar);
    mToolbarLayout.addWidget(&mOctaveLabel);
    mToolbarLayout.addWidget(&mOctaveSpin);
    mToolbarLayout.addWidget(&mEditStepLabel);
    mToolbarLayout.addWidget(&mEditStepSpin);
    mToolbarLayout.addWidget(&mLoopPatternCheck);
    mToolbarLayout.addWidget(&mFollowModeCheck);
    mToolbarLayout.addWidget(&mKeyRepeatCheck);
    mToolbarLayout.addStretch(1);

    for (auto &line : mLines) {
        line.setFrameShape(QFrame::VLine);
        line.setFrameShadow(QFrame::Sunken);
    }

    mSettingsLayout.addWidget(&mRowsPerBeatLabel,       0, 0);
    mSettingsLayout.addWidget(&mRowsPerBeatSpin,        0, 1);
    mSettingsLayout.addWidget(&mLines[0],               0, 2, 2, 1);
    mSettingsLayout.addWidget(&mSpeedLabel,             0, 3);
    mSettingsLayout.addWidget(&mSpeedSpin,              0, 4);
    mSettingsLayout.addWidget(&mSpeedActualLabel,       0, 5);
    mSettingsLayout.addWidget(&mLines[1],               0, 6, 2, 1);
    mSettingsLayout.addWidget(&mPatternsLabel,          0, 7);
    mSettingsLayout.addWidget(&mPatternsSpin,           0, 8);
    mSettingsLayout.addWidget(&mLines[2],               0, 9, 2, 1);
    mSettingsLayout.addWidget(&mInstrumentCombo,        0, 10);
    auto spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mSettingsLayout.addItem(spacer,                     0, 11);

    mSettingsLayout.addWidget(&mRowsPerMeasureLabel,    1, 0);
    mSettingsLayout.addWidget(&mRowsPerMeasureSpin,     1, 1);
    mSettingsLayout.addWidget(&mTempoLabel,             1, 3);
    mSettingsLayout.addWidget(&mTempoSpin,              1, 4);
    mSettingsLayout.addWidget(&mTempoActualLabel,       1, 5);
    mSettingsLayout.addWidget(&mPatternSizeLabel,       1, 7);
    mSettingsLayout.addWidget(&mPatternSizeSpin,        1, 8);
    mSettingsLayout.addWidget(&mSetInstrumentCheck,     1, 10);

    mSettingsLayout.setColumnStretch(10, 1);
    mSettingsLayout.setColumnStretch(11, 1);
    

    mControlsLayout.addLayout(&mToolbarLayout);
    mControlsLayout.addLayout(&mSettingsLayout);
    mControls.setLayout(&mControlsLayout);

    mGridLayout.setMargin(0);
    mGridLayout.setSpacing(0);
    mGridLayout.addWidget(&mGridHeader,     0, 0);
    mGridLayout.addWidget(&mGrid,           1, 0);
    mGridLayout.addWidget(&mVScroll,        0, 1, 2, 1);
    mGridLayout.addWidget(&mHScroll,        2, 0);
    mGridFrame.setLayout(&mGridLayout);
    mGridFrame.setFrameStyle(QFrame::StyledPanel);


    mLayout.setMargin(0);
    mLayout.setSpacing(0);
    mLayout.addWidget(&mControls);
    mLayout.addWidget(&mGridFrame, 1);
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

    connect(&mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::octaveChanged);

    mOctaveSpin.setValue(4);
    mFollowModeCheck.setCheckState(Qt::Checked);
    mKeyRepeatCheck.setCheckState(Qt::Checked);
    mSetInstrumentCheck.setChecked(true);

    connect(&mTrackerActions.record, &QAction::toggled, this,
        [this](bool checked) {
            mDocument->patternModel().setRecord(checked);
        });

    connect(&mVScroll, &QScrollBar::valueChanged, this,
        [this](int value) {
            mDocument->patternModel().setCursorRow(value);
        });
    connect(&mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);

    connect(&mHScroll, &QScrollBar::valueChanged, this,
        [this](int value) {
            mDocument->patternModel().setCursorColumn(value);
        });
    connect(&mHScroll, &QScrollBar::actionTriggered, this, &PatternEditor::hscrollAction);

    mRowsPerBeatSpin.setRange(1, 255);
    mRowsPerMeasureSpin.setRange(1, 255);
    mSpeedSpin.setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin.setDisplayIntegerBase(16);
    mSpeedSpin.setPrefix("0x");
    mTempoSpin.setRange(1, 10000);
    mTempoSpin.setValue(150);
    mTempoSpin.setSuffix(" BPM");
    mPatternsSpin.setRange(1, trackerboy::MAX_PATTERNS);
    mPatternSizeSpin.setRange(1, 256);

    mSpeedActualLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mTempoActualLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // a change in the rows per peat setting requires a recalculation in the actual tempo label
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::updateTempoLabel);
    connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::speedChanged);
    connect(&mTempoSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::tempoChanged);
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerBeatSpin, &QSpinBox::setMaximum);

    connect(&mSetInstrumentCheck, &QCheckBox::stateChanged, this, &PatternEditor::enableAutoInstrument);
    connect(&mInstrumentCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &PatternEditor::setAutoInstrument);

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
    
    auto &patternModel = mDocument->patternModel();
    
    // navigation keys / non-edit keys
    // these keys also ignore the key repetition setting (they always repeat)
    switch (key) {
        case Qt::Key_Left:
            patternModel.moveCursorColumn(-1);
            return;
        case Qt::Key_Right:
            patternModel.moveCursorColumn(1);
            return;
        case Qt::Key_Up:
            patternModel.moveCursorRow(-1);
            return;
        case Qt::Key_Down:
            patternModel.moveCursorRow(1);
            return;
        case Qt::Key_PageDown:
            patternModel.moveCursorRow(mPageStep);
            return;
        case Qt::Key_PageUp:
            patternModel.moveCursorRow(-mPageStep);
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
        patternModel.moveCursorRow(mEditStepSpin.value());
        
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
        mDocument->patternModel().moveCursorRow(amount);
    }

    evt->accept();
}

void PatternEditor::hscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mDocument->patternModel().moveCursorColumn(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mDocument->patternModel().moveCursorColumn(-1);
            break;
        default:
            break;
    }
}

void PatternEditor::vscrollAction(int action) {
    switch (action) {
        case QAbstractSlider::SliderSingleStepAdd:
            mDocument->patternModel().moveCursorRow(1);
            break;
        case QAbstractSlider::SliderSingleStepSub:
            mDocument->patternModel().moveCursorRow(-1);
            break;
        default:
            break;
    }
}

void PatternEditor::setDocument(ModuleDocument *doc) {
    if (mDocument != nullptr) {
        // save state to document
        auto &state = mDocument->state();
        state.octave = mOctaveSpin.value();
        state.editStep = mEditStepSpin.value();
        state.loopPattern = mLoopPatternCheck.isChecked();
        state.followMode = mFollowModeCheck.isChecked();
        state.keyRepetition = mKeyRepeatCheck.isChecked();
        state.autoInstrument = mSetInstrumentCheck.isChecked();
        state.autoInstrumentIndex = mInstrumentCombo.currentIndex();

        auto &orderModel = mDocument->orderModel();
        orderModel.disconnect(this);
        mPatternsSpin.disconnect(&orderModel);

        auto &songModel = mDocument->songModel();
        mRowsPerBeatSpin.disconnect(&songModel);
        mRowsPerMeasureSpin.disconnect(&songModel);
        mSpeedSpin.disconnect(&songModel);
        mPatternSizeSpin.disconnect(&songModel);

        auto &patternModel = mDocument->patternModel();
        patternModel.disconnect(this);
        patternModel.disconnect(&mTrackerActions.record);
        patternModel.disconnect(&mVScroll);
        patternModel.disconnect(&mHScroll);
        patternModel.disconnect(&mFollowModeCheck);
        
    }

    mDocument = doc;
    mGrid.setDocument(doc);
    mGridHeader.setDocument(doc);
    if (doc) {
        // restore state from document
        auto const& state = doc->state();
        mOctaveSpin.setValue(state.octave);
        mEditStepSpin.setValue(state.editStep);
        mLoopPatternCheck.setChecked(state.loopPattern);
        mFollowModeCheck.setChecked(state.followMode);
        mKeyRepeatCheck.setChecked(state.keyRepetition);
        mSetInstrumentCheck.setChecked(state.autoInstrument);
        mInstrumentCombo.setModel(&doc->instrumentModel());
        mInstrumentCombo.setCurrentIndex(state.autoInstrumentIndex);

        auto &orderModel = doc->orderModel();
        mPatternsSpin.setValue(orderModel.rowCount());
        
        connect(&orderModel, &OrderModel::rowsInserted, this, &PatternEditor::updatePatternsSpin);
        connect(&orderModel, &OrderModel::rowsRemoved, this, &PatternEditor::updatePatternsSpin);
        connect(&mPatternsSpin, qOverload<int>(&QSpinBox::valueChanged), &orderModel, &OrderModel::setPatternCount);
        

        auto &songModel = doc->songModel();
        mRowsPerBeatSpin.setValue(songModel.rowsPerBeat());
        mRowsPerMeasureSpin.setValue(songModel.rowsPerMeasure());
        mSpeedSpin.setValue(songModel.speed());
        mPatternSizeSpin.setValue(songModel.patternSize());
        connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerBeat);
        connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerMeasure);
        connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setSpeed);
        connect(&mPatternSizeSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setPatternSize);
    
        auto &patternModel = doc->patternModel();
        mTrackerActions.record.setChecked(patternModel.isRecording());
        connect(&patternModel, &PatternModel::recordingChanged, &mTrackerActions.record, &QAction::setChecked);
        connect(&patternModel, &PatternModel::cursorRowChanged, &mVScroll, &QScrollBar::setValue);
        connect(&patternModel, &PatternModel::cursorColumnChanged, &mHScroll, &QScrollBar::setValue);
        
        mVScroll.setMaximum((int)patternModel.currentPattern().totalRows() - 1);
        mVScroll.setValue(patternModel.cursorRow());
        mHScroll.setValue(patternModel.cursorColumn());
        connect(&patternModel, &PatternModel::patternSizeChanged, this,
            [this](int rows) {
                mVScroll.setMaximum(rows - 1);
            });

        mFollowModeCheck.setChecked(patternModel.isFollowing());
        connect(&mFollowModeCheck, &QCheckBox::toggled, &patternModel, &PatternModel::setFollowing);

    }
}

void PatternEditor::onCut() {

}

void PatternEditor::onCopy() {
    
}

void PatternEditor::onPaste() {
    
}

void PatternEditor::onPasteMix() {
    
}

void PatternEditor::onDelete() {
    
}

void PatternEditor::onSelectAll() {
    
}

void PatternEditor::onIncreaseNote() {

}

void PatternEditor::onDecreaseNote() {

}

void PatternEditor::onIncreaseOctave() {

}

void PatternEditor::onDecreaseOctave() {

}

void PatternEditor::onReverse() {
    
}


void PatternEditor::updatePatternsSpin(QModelIndex const& index, int first, int last) {
    Q_UNUSED(index)
    Q_UNUSED(first)
    Q_UNUSED(last)

    mPatternsSpin.setValue(mDocument->orderModel().rowCount());
}

void PatternEditor::updateTempoLabel() {
    // human-readable speed value
    float speed = trackerboy::speedToFloat((trackerboy::Speed)mSpeedSpin.value());
    float tempo = calcActualTempo(speed);
    setTempoLabel(tempo);
    mTempoSpin.setValue((int)roundf(tempo));
}

void PatternEditor::setTempoLabel(float tempo) {
    mTempoActualLabel.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
}

float PatternEditor::calcActualTempo(float speed) {
    // actual tempo value
    // tempo = (framerate * 60) / (speed * rpb)
    // convert fixed point to floating point
    return (mDocument->mod().framerate() * 60.0f) / (speed * mRowsPerBeatSpin.value());
}

void PatternEditor::speedChanged(int value) {

    auto speed = trackerboy::speedToFloat((trackerboy::Speed)value);
    auto tempo = calcActualTempo(speed);
    setTempoLabel(tempo);
    mSpeedActualLabel.setText(tr("%1 FPR").arg(speed, 0, 'f', 3));

    if (!mSpeedLock) {
        mSpeedLock = true;

        // convert the speed to tempo and set the tempo spin
        mTempoSpin.setValue((int)roundf(tempo));

        mSpeedLock = false;
    }
}

void PatternEditor::tempoChanged(int value) {
    if (!mSpeedLock) {
        mSpeedLock = true;

        // convert tempo to speed and set in the speed spin
        // speed = (framerate * 60) / (tempo * rpb)
        float speed = (mDocument->mod().framerate() * 60.0f) / (value * mRowsPerBeatSpin.value());
        mSpeedSpin.setValue((int)round(speed * (1 << trackerboy::SPEED_FRACTION_BITS)));

        mSpeedLock = false;
    }
}

void PatternEditor::enableAutoInstrument(bool enabled) {
    if (enabled) {
        setAutoInstrument(mInstrumentCombo.currentIndex());
    } else {
        mInstrument.reset();
    }
    mInstrumentCombo.setEnabled(enabled);
}

void PatternEditor::setAutoInstrument(int index) {
    if (index == -1) {
        mInstrument = 0;
    } else {
        mInstrument = mDocument->instrumentModel().id(index);
    }
}