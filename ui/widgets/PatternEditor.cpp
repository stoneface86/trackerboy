
#include "widgets/PatternEditor.hpp"

#include "misc/utils.hpp"
#include "widgets/grid/layout.hpp"

#include <QDialogButtonBox>
#include <QClipboard>
#include <QDialog>
#include <QGuiApplication>
#include <QGridLayout>
#include <QtDebug>

#include <cmath>

static std::optional<trackerboy::EffectType> keyToEffectType(int const key) {
    switch (key) {
        case Qt::Key_B:
            return trackerboy::EffectType::patternGoto;
        case Qt::Key_C:
            return trackerboy::EffectType::patternHalt;
        case Qt::Key_D:
            return trackerboy::EffectType::patternSkip;
        case Qt::Key_F:
            return trackerboy::EffectType::setTempo;
        case Qt::Key_T:
            return trackerboy::EffectType::sfx;
        case Qt::Key_E:
            return trackerboy::EffectType::setEnvelope;
        case Qt::Key_V:
            return trackerboy::EffectType::setTimbre;
        case Qt::Key_I:
            return trackerboy::EffectType::setPanning;
        case Qt::Key_H:
            return trackerboy::EffectType::setSweep;
        case Qt::Key_S:
            return trackerboy::EffectType::delayedCut;
        case Qt::Key_G:
            return trackerboy::EffectType::delayedNote;
        case Qt::Key_L:
            return trackerboy::EffectType::lock;
        case Qt::Key_0:
            return trackerboy::EffectType::arpeggio;
        case Qt::Key_1:
            return trackerboy::EffectType::pitchUp;
        case Qt::Key_2:
            return trackerboy::EffectType::pitchDown;
        case Qt::Key_3:
            return trackerboy::EffectType::autoPortamento;
        case Qt::Key_4:
            return trackerboy::EffectType::vibrato;
        case Qt::Key_5:
            return trackerboy::EffectType::vibratoDelay;
        case Qt::Key_P:
            return trackerboy::EffectType::tuning;
        case Qt::Key_Q:
            return trackerboy::EffectType::noteSlideUp;
        case Qt::Key_R:
            return trackerboy::EffectType::noteSlideDown;
        case Qt::Key_Delete:
            return trackerboy::EffectType::noEffect;
        default:
            return std::nullopt;
    }
}

static std::optional<uint8_t> keyToHex(int const key) {
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return (uint8_t)(key - Qt::Key_0);
    } else if (key >= Qt::Key_A && key <= Qt::Key_F) {
        return (uint8_t)(key - Qt::Key_A + 0xA);
    } else {
        return std::nullopt;
    }
}

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
    // mLayout(),
    // mControls(),
    // mControlsLayout(),
    
    // mSettingsLayout(),
    // mRowsPerBeatLabel(tr("Rows per beat:")),
    // mRowsPerBeatSpin(),
    // mSpeedLabel(tr("Speed:")),
    // mSpeedSpin(),
    // mSpeedActualLabel(),
    // mPatternsLabel(tr("Patterns:")),
    // mPatternsSpin(),
    // mInstrumentCombo(),
    // mRowsPerMeasureLabel(tr("Rows per measure:")),
    // mRowsPerMeasureSpin(),
    // mTempoLabel(tr("Tempo:")),
    // mTempoSpin(),
    // mTempoActualLabel(),
    // mPatternSizeLabel(tr("Pattern size:")),
    // mPatternSizeSpin(),
    // mSetInstrumentCheck(tr("Set instrument column")),
    // mLines(),
    // mGridFrame(),
    mLayout(),
    mGridHeader(),
    mGrid(mGridHeader),
    mHScroll(Qt::Horizontal),
    mVScroll(Qt::Vertical),
    mWheel(0),
    mPageStep(4),
    mSpeedLock(false),
    mScrollLock(false),
    mClipboard(),
    mInstrument()
{

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setFocusPolicy(Qt::StrongFocus);

    mHScroll.setMinimum(0);
    mHScroll.setMaximum(PatternCursor::MAX_COLUMNS * PatternCursor::MAX_TRACKS - 1);
    mHScroll.setPageStep(1);

    // for (auto &line : mLines) {
    //     line.setFrameShape(QFrame::VLine);
    //     line.setFrameShadow(QFrame::Sunken);
    // }

    // mSettingsLayout.addWidget(&mRowsPerBeatLabel,       0, 0);
    // mSettingsLayout.addWidget(&mRowsPerBeatSpin,        0, 1);
    // mSettingsLayout.addWidget(&mLines[0],               0, 2, 2, 1);
    // mSettingsLayout.addWidget(&mSpeedLabel,             0, 3);
    // mSettingsLayout.addWidget(&mSpeedSpin,              0, 4);
    // mSettingsLayout.addWidget(&mSpeedActualLabel,       0, 5);
    // mSettingsLayout.addWidget(&mLines[1],               0, 6, 2, 1);
    // mSettingsLayout.addWidget(&mPatternsLabel,          0, 7);
    // mSettingsLayout.addWidget(&mPatternsSpin,           0, 8);


    // mSettingsLayout.addWidget(&mRowsPerMeasureLabel,    1, 0);
    // mSettingsLayout.addWidget(&mRowsPerMeasureSpin,     1, 1);
    // mSettingsLayout.addWidget(&mTempoLabel,             1, 3);
    // mSettingsLayout.addWidget(&mTempoSpin,              1, 4);
    // mSettingsLayout.addWidget(&mTempoActualLabel,       1, 5);
    // mSettingsLayout.addWidget(&mPatternSizeLabel,       1, 7);
    // mSettingsLayout.addWidget(&mPatternSizeSpin,        1, 8);

    // mSettingsLayout.setColumnStretch(10, 1);
    // mSettingsLayout.setColumnStretch(11, 1);
    
    // mControlsLayout.addLayout(&mSettingsLayout);
    // mControls.setLayout(&mControlsLayout);

    // mGridLayout.setMargin(0);
    // mGridLayout.setSpacing(0);
    mLayout.addWidget(&mGridHeader,     0, 0);
    mLayout.addWidget(&mGrid,           1, 0);
    mLayout.addWidget(&mVScroll,        0, 1, 2, 1);
    mLayout.addWidget(&mHScroll,        2, 0);
    mLayout.setSpacing(0);
    mLayout.setMargin(0);
    setLayout(&mLayout);
    // mGridFrame.setLayout(&mGridLayout);
    // mGridFrame.setFrameStyle(QFrame::StyledPanel);


    // mLayout.setMargin(0);
    // mLayout.setSpacing(0);
    // mLayout.addWidget(&mControls);
    // mLayout.addWidget(&mGridFrame, 1);
    // setLayout(&mLayout);

    // mEditStepSpin.setRange(0, 256);
    // mEditStepSpin.setValue(1);
    // mOctaveSpin.setRange(2, 8);



    //connect(&mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::octaveChanged);

    //mOctaveSpin.setValue(4);
    //mSetInstrumentCheck.setChecked(true);

    

    // mRowsPerBeatSpin.setRange(1, 255);
    // mRowsPerMeasureSpin.setRange(1, 255);
    // mSpeedSpin.setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    // mSpeedSpin.setDisplayIntegerBase(16);
    // mSpeedSpin.setPrefix("0x");
    // mTempoSpin.setRange(1, 10000);
    // mTempoSpin.setValue(150);
    // mTempoSpin.setSuffix(" BPM");
    // mPatternsSpin.setRange(1, trackerboy::MAX_PATTERNS);
    // mPatternSizeSpin.setRange(1, 256);

    // mSpeedActualLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // mTempoActualLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // connections

    connect(&mVScroll, &QScrollBar::actionTriggered, this, &PatternEditor::vscrollAction);
    connect(&mHScroll, &QScrollBar::actionTriggered, this, &PatternEditor::hscrollAction);

    // a change in the rows per peat setting requires a recalculation in the actual tempo label
    // connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::updateTempoLabel);
    // connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::speedChanged);
    // connect(&mTempoSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PatternEditor::tempoChanged);
    // connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    // connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerBeatSpin, &QSpinBox::setMaximum);

    // connect(&mSetInstrumentCheck, &QCheckBox::stateChanged, this, &PatternEditor::enableAutoInstrument);
    // connect(&mInstrumentCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &PatternEditor::setAutoInstrument);

    
    // buddies
    // mOctaveLabel.setBuddy(&mOctaveSpin);
    // mEditStepLabel.setBuddy(&mEditStepSpin);
    // mRowsPerBeatLabel.setBuddy(&mRowsPerBeatSpin);
    // mSpeedLabel.setBuddy(&mSpeedSpin);
    // mPatternsLabel.setBuddy(&mPatternsSpin);
    // mRowsPerMeasureLabel.setBuddy(&mRowsPerMeasureSpin);
    // mTempoLabel.setBuddy(&mTempoSpin);
    // mPatternSizeLabel.setBuddy(&mPatternSizeSpin);

    setDocument(nullptr);
    
    // mInputToolbar.setWindowTitle(tr("Input"));
    // mInputToolbar.setIconSize(QSize(16, 16));
    // mInputToolbar.setObjectName(QStringLiteral("mInputTitle"));
    // mInputToolbar.addWidget(&mOctaveLabel);
    // mInputToolbar.addWidget(&mOctaveSpin);
    // mInputToolbar.addWidget(&mEditStepLabel);
    // mInputToolbar.addWidget(&mEditStepSpin);
    // mInputToolbar.addAction(&mActions.keyRepetition);
    // mInputToolbar.setStyleSheet(QStringLiteral("spacing: 8px;"));

    // mInstrumentToolbar.setWindowTitle(tr("Instrument"));
    // mInstrumentToolbar.setIconSize(QSize(16, 16));
    // mInstrumentToolbar.setObjectName(QStringLiteral("mInstrumentToolbar"));
    // mInstrumentToolbar.addWidget(&mInstrumentCombo);
    // mInstrumentCombo.setMinimumWidth(224);

}

PatternGrid& PatternEditor::grid() {
    return mGrid;
}

void PatternEditor::setColors(ColorTable const& colors) {
    mGridHeader.setColors(colors);
    mGrid.setColors(colors);
}

bool PatternEditor::event(QEvent *evt)  {
    if (evt->type() == QEvent::KeyPress) {
        auto keyEvt = static_cast<QKeyEvent*>(evt);
        auto key = keyEvt->key();

        // intercept tabs for pattern navigation
        if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
            // intercept tab presses
            int amount = key == Qt::Key_Backtab ? -1 : 1;
            mDocument->patternModel().moveCursorTrack(amount);
            return true;
        }
    }

    return QWidget::event(evt);
}

void PatternEditor::focusInEvent(QFocusEvent *evt) {
    Q_UNUSED(evt)
    mGrid.setEditorFocus(true);
}

void PatternEditor::focusOutEvent(QFocusEvent *evt) {
    Q_UNUSED(evt)
    mGrid.setEditorFocus(false);
}

void PatternEditor::keyPressEvent(QKeyEvent *evt) {

    auto const modifiers = evt->modifiers();
    int const key = evt->key();

    auto const controlDown = modifiers.testFlag(Qt::ControlModifier);
    auto const shiftDown = modifiers.testFlag(Qt::ShiftModifier);

    auto selectionMode = shiftDown ? PatternModel::SelectionModify : PatternModel::SelectionRemove;
    
    auto &patternModel = mDocument->patternModel();
    auto &orderModel = mDocument->orderModel();

    // Up/Down/Left/Right - move cursor by 1 (also selects if shift is down)
    // PgUp/PgDn - move cursor by page step (also selects if shift is down)
    // Ctrl+Up/Ctrl+Down - select current instrument
    // Ctrl+Left/Ctrl+Right - select current pattern
    // Tab/Shift-Tab - move cursor to next/previous track
    // Space - toggles record mode
    // Numpad / * - decrease or increase octave
    
    // navigation keys / non-edit keys
    // these keys also ignore the key repetition setting (they always repeat)
    switch (key) {
        case Qt::Key_Left:
            if (controlDown) {
                orderModel.selectPattern(orderModel.currentPattern() - 1);
            } else {
                patternModel.moveCursorColumn(-1, selectionMode);

            }
            return;
        case Qt::Key_Right:
            if (controlDown) {
                orderModel.selectPattern(orderModel.currentPattern() + 1);
            } else {
                patternModel.moveCursorColumn(1, selectionMode);
            }
            return;
        case Qt::Key_Up:
            if (controlDown) {
                emit nextInstrument();
            } else {
                patternModel.moveCursorRow(-1, selectionMode);
            }
            return;
        case Qt::Key_Down:
            if (controlDown) {
                emit previousInstrument();
            } else {
                patternModel.moveCursorRow(1, selectionMode);
            }
            return;
        case Qt::Key_PageDown:
            patternModel.moveCursorRow(mPageStep, selectionMode);
            return;
        case Qt::Key_PageUp:
            patternModel.moveCursorRow(-mPageStep, selectionMode);
            return;
        case Qt::Key_Space:
            patternModel.setRecord(!patternModel.isRecording());
            return;
        case Qt::Key_Asterisk:
            if (modifiers.testFlag(Qt::KeypadModifier)) {
                emit changeOctave(mPianoIn.octave() + 1);
                return;
            }
            break;
        case Qt::Key_Slash:
            if (modifiers.testFlag(Qt::KeypadModifier)) {
                emit changeOctave(mPianoIn.octave() - 1);
                return;
            }
            break;
    }

    if (evt->isAutoRepeat() && !mDocument->keyRepetition()) {
        QWidget::keyPressEvent(evt);
        return; // key repetition disabled, ignore this event
    }

    // ignore this event if CTRL is present (conflicts with shortcuts)
    if (controlDown) {
        QWidget::keyPressEvent(evt);
        return;
    }

    auto const recording = patternModel.isRecording();
    bool validKey = false;

    switch (patternModel.cursorColumn()) {
        case PatternCursor::ColumnNote: {
            auto note = mPianoIn.keyToNote(key);

            if (note) {
                if (*note != trackerboy::NOTE_CUT) {
                    mPreviewKey = key;
                    emit previewNote(*note);
                }
                if (recording) {
                    patternModel.setNote(note, mInstrument);
                }
                validKey = true;
            }

            break;
        }
        case PatternCursor::ColumnEffect1Type:
        case PatternCursor::ColumnEffect2Type:
        case PatternCursor::ColumnEffect3Type:
        {
            // check if the key pressed is a valid effect type
            auto effectType = keyToEffectType(key);
            if (effectType) {
                if (recording) {
                    patternModel.setEffectType(*effectType);
                }
                validKey = true;
            }
            break;
        }
        case PatternCursor::ColumnInstrumentHigh:
        case PatternCursor::ColumnInstrumentLow: {
            auto hex = keyToHex(key);
            if (hex) {
                if (recording) {
                    patternModel.setInstrument(hex);
                }
                validKey = true;
            }
            break;
        }
        case PatternCursor::ColumnEffect1ArgHigh:
        case PatternCursor::ColumnEffect1ArgLow:
        case PatternCursor::ColumnEffect2ArgHigh:
        case PatternCursor::ColumnEffect2ArgLow:
        case PatternCursor::ColumnEffect3ArgHigh:
        case PatternCursor::ColumnEffect3ArgLow:
        {
            // check if the key pressed is a hex number
            auto hex = keyToHex(key);
            if (hex) {
                if (recording) {
                    patternModel.setEffectParam(*hex);
                }
                validKey = true;
            }
            
        }
        break;
    }

    if (validKey) {
        if (recording) {
            stepDown();
        }
    } else {
        // invalid key or edit mode is off, let QWidget handle it
        QWidget::keyPressEvent(evt);
    }
    
}

void PatternEditor::keyReleaseEvent(QKeyEvent *evt) {
    if (evt->key() == mPreviewKey) {
        mPreviewKey = Qt::Key_unknown;
        emit stopNotePreview();
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
        //state.octave = mOctaveSpin.value();
        //state.editStep = mEditStepSpin.value();
        //state.autoInstrument = mSetInstrumentCheck.isChecked();
        //state.autoInstrumentIndex = mInstrumentCombo.currentIndex();

        for (auto const& conn : mConnections) {
            disconnect(conn);
        }
        mConnections.clear();

    }

    auto const hasDocument = doc != nullptr;
    mDocument = doc;
    mGrid.setDocument(doc);
    mGridHeader.setDocument(doc);
    if (hasDocument) {
        // restore state from document
        auto const& state = doc->state();
        //mOctaveSpin.setValue(state.octave);
        //mEditStepSpin.setValue(state.editStep);
        //mSetInstrumentCheck.setChecked(state.autoInstrument);
        //mInstrumentCombo.setModel(&doc->instrumentModel());
        //mInstrumentCombo.setCurrentIndex(state.autoInstrumentIndex);

        auto &orderModel = doc->orderModel();
        //mPatternsSpin.setValue(orderModel.rowCount());
        
        //mConnections.append(connect(&orderModel, &OrderModel::rowsInserted, this, &PatternEditor::updatePatternsSpin));
        //mConnections.append(connect(&orderModel, &OrderModel::rowsRemoved, this, &PatternEditor::updatePatternsSpin));
        //mConnections.append(connect(&mPatternsSpin, qOverload<int>(&QSpinBox::valueChanged), &orderModel, &OrderModel::setPatternCount));
        

        //auto &songModel = doc->songModel();
        //mRowsPerBeatSpin.setValue(songModel.rowsPerBeat());
        //mRowsPerMeasureSpin.setValue(songModel.rowsPerMeasure());
        //mSpeedSpin.setValue(songModel.speed());
        //mPatternSizeSpin.setValue(songModel.patternSize());
        //mConnections.append(connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerBeat));
        //mConnections.append(connect(&mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setRowsPerMeasure));
        //mConnections.append(connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setSpeed));
        //mConnections.append(connect(&mPatternSizeSpin, qOverload<int>(&QSpinBox::valueChanged), &songModel, &SongModel::setPatternSize));
    
        auto &patternModel = doc->patternModel();
        // scrollbars
        mVScroll.setMaximum((int)patternModel.currentPattern().totalRows() - 1);
        updateScrollbars(PatternModel::CursorRowChanged | PatternModel::CursorColumnChanged);
        mConnections.append(connect(&mVScroll, &QScrollBar::valueChanged, &patternModel, &PatternModel::setCursorRow));
        mConnections.append(connect(&mHScroll, &QScrollBar::valueChanged, this, &PatternEditor::setCursorFromHScroll));
        mConnections.append(connect(&patternModel, &PatternModel::cursorChanged, this, &PatternEditor::updateScrollbars));

        mConnections.append(connect(&patternModel, &PatternModel::patternSizeChanged, this,
            [this](int rows) {
                mVScroll.setMaximum(rows - 1);
            }));

        //mFollowModeCheck.setChecked(patternModel.isFollowing());
        //mConnections.append(connect(&mFollowModeCheck, &QCheckBox::toggled, &patternModel, &PatternModel::setFollowing));

        // mConnections.append(connect(&patternModel, &PatternModel::selectionChanged, this,
        //     [this]() {
        //         bool hasSelection = static_cast<PatternModel*>(sender())->hasSelection();
        //         mActions.copy.setEnabled(hasSelection);
        //         mActions.cut.setEnabled(hasSelection);
        //         mActions.reverse.setEnabled(hasSelection);
        //     }));
    }
    

}

void PatternEditor::setInstrument(int index) {
    if (index == 0) {
        mInstrument.reset();
    } else {
        mInstrument = mDocument->instrumentModel().id(index - 1);
    }
}

void PatternEditor::cut() {
    copy();
    mDocument->patternModel().deleteSelection();
}

void PatternEditor::copy() {
    auto clip = mDocument->patternModel().clip();
    mClipboard.setClip(clip);
}

void PatternEditor::paste() {
    pasteImpl(false);
}

void PatternEditor::pasteMix() {
    pasteImpl(true);
}

void PatternEditor::pasteImpl(bool mix) {
    if (mClipboard.hasClip()) {
        auto &clip = mClipboard.clip();
        mDocument->patternModel().paste(clip, mix);
    }
}

void PatternEditor::erase() {

    mDocument->patternModel().deleteSelection();
    if (!mDocument->patternModel().hasSelection()) {
        stepDown();
    }
}

void PatternEditor::selectAll() {
    mDocument->patternModel().selectAll();
}

void PatternEditor::increaseNote() {
    mDocument->patternModel().transpose(1);
}

void PatternEditor::decreaseNote() {
    mDocument->patternModel().transpose(-1);
}

void PatternEditor::increaseOctave() {
    mDocument->patternModel().transpose(12);
}

void PatternEditor::decreaseOctave() {
    mDocument->patternModel().transpose(-12);
}

void PatternEditor::transpose() {
    QDialog dialog(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    dialog.setWindowTitle(tr("Transpose"));

    QVBoxLayout layout;
        QLabel label(tr("Enter transpose amount (semitones):"));
        QSpinBox transposeSpin;
        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    layout.addWidget(&label);
    layout.addWidget(&transposeSpin);
    layout.addWidget(&buttons);
    layout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    dialog.setLayout(&layout);

    transposeSpin.setRange(-trackerboy::NOTE_LAST, trackerboy::NOTE_LAST);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        mDocument->patternModel().transpose(transposeSpin.value());
    }
}

void PatternEditor::reverse() {
    mDocument->patternModel().reverse();
}


void PatternEditor::updatePatternsSpin(QModelIndex const& index, int first, int last) {
    Q_UNUSED(index)
    Q_UNUSED(first)
    Q_UNUSED(last)

    //mPatternsSpin.setValue(mDocument->orderModel().rowCount());
}

void PatternEditor::updateTempoLabel() {
    // human-readable speed value
    //float speed = trackerboy::speedToFloat((trackerboy::Speed)mSpeedSpin.value());
    //float tempo = calcActualTempo(speed);
    //setTempoLabel(tempo);
    //mTempoSpin.setValue((int)roundf(tempo));
}

void PatternEditor::setTempoLabel(float tempo) {
    //mTempoActualLabel.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
}

float PatternEditor::calcActualTempo(float speed) {
    // actual tempo value
    // tempo = (framerate * 60) / (speed * rpb)
    // convert fixed point to floating point
    //return (mDocument->mod().framerate() * 60.0f) / (speed * mRowsPerBeatSpin.value());
    return 0.0f;
}

void PatternEditor::speedChanged(int value) {

    // auto speed = trackerboy::speedToFloat((trackerboy::Speed)value);
    // auto tempo = calcActualTempo(speed);
    // setTempoLabel(tempo);
    // mSpeedActualLabel.setText(tr("%1 FPR").arg(speed, 0, 'f', 3));

    // if (!mSpeedLock) {
    //     mSpeedLock = true;

    //     // convert the speed to tempo and set the tempo spin
    //     mTempoSpin.setValue((int)roundf(tempo));

    //     mSpeedLock = false;
    // }
}

void PatternEditor::tempoChanged(int value) {
    // if (!mSpeedLock) {
    //     mSpeedLock = true;

    //     // convert tempo to speed and set in the speed spin
    //     // speed = (framerate * 60) / (tempo * rpb)
    //     float speed = (mDocument->mod().framerate() * 60.0f) / (value * mRowsPerBeatSpin.value());
    //     mSpeedSpin.setValue((int)round(speed * (1 << trackerboy::SPEED_FRACTION_BITS)));

    //     mSpeedLock = false;
    // }
}

void PatternEditor::enableAutoInstrument(bool enabled) {
    // if (enabled) {
    //     setAutoInstrument(mInstrumentCombo.currentIndex());
    // } else {
    //     mInstrument.reset();
    // }
    // mInstrumentCombo.setEnabled(enabled);
}

void PatternEditor::setAutoInstrument(int index) {
    // if (index == -1) {
    //     mInstrument = 0;
    // } else {
    //     mInstrument = mDocument->instrumentModel().id(index);
    // }
}

void PatternEditor::stepDown() {
    mDocument->patternModel().moveCursorRow(mDocument->editStep());
}

void PatternEditor::updateScrollbars(PatternModel::CursorChangeFlags flags) {

    if (!mScrollLock) {
        mScrollLock = true;

        auto cursor = mDocument->patternModel().cursor();
        if (flags.testFlag(PatternModel::CursorRowChanged)) {
            mVScroll.setValue(cursor.row);
        }

        if (flags & (PatternModel::CursorTrackChanged | PatternModel::CursorColumnChanged)) {
            mHScroll.setValue(cursor.track * PatternCursor::MAX_COLUMNS + cursor.column);
        }

        mScrollLock = false;
    }
}

void PatternEditor::setCursorFromHScroll(int value) {
    if (!mScrollLock) {
        mScrollLock = true;

        auto &patternModel = mDocument->patternModel();
        auto cursor = patternModel.cursor();
        cursor.column = value % PatternCursor::MAX_COLUMNS;
        cursor.track = value / PatternCursor::MAX_COLUMNS;
        patternModel.setCursor(cursor);

        mScrollLock = false;
    }
}
