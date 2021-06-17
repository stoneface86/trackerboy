
#pragma once

#include "core/clipboard/PatternClipboard.hpp"
#include "core/model/OrderModel.hpp"
#include "core/PianoInput.hpp"
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/PatternGridHeader.hpp"
#include "widgets/CustomSpinBox.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QScrollBar>
#include <QSpinBox>
#include <QToolBar>
#include <QVector>

#include <array>
#include <optional>


class PatternEditor : public QFrame {

    Q_OBJECT

public:

    

    struct Actions {
        QAction cut;
        QAction copy;
        QAction paste;
        QAction pasteMix;
        QAction delete_;
        QAction selectAll;
        QAction noteIncrease;
        QAction noteDecrease;
        QAction octaveIncrease;
        QAction octaveDecrease;
        QAction transpose;
        QAction reverse;

    };

    explicit PatternEditor(PianoInput const& input, QWidget *parent = nullptr);
    ~PatternEditor() = default;

    PatternGrid& grid();

    Actions& menuActions();

    void setupMenu(QMenu &menu);

    void setColors(ColorTable const& colors);

    virtual bool event(QEvent *evt) override;

signals:
    void octaveChanged(int octave);

    void previewNote(quint8 note);

    void stopNotePreview();

public slots:

    void setDocument(ModuleDocument *doc);

    void onCut();

    void onCopy();

    void onPaste();

    void onPasteMix();

    void onDelete();

    void onSelectAll();

    void onIncreaseNote();

    void onDecreaseNote();

    void onIncreaseOctave();

    void onDecreaseOctave();

    void onTranspose();

    void onReverse();

protected:

    virtual void focusInEvent(QFocusEvent *evt) override;

    virtual void focusOutEvent(QFocusEvent *evt) override;

    virtual void keyPressEvent(QKeyEvent *evt) override;

    virtual void keyReleaseEvent(QKeyEvent *evt) override;

    virtual void wheelEvent(QWheelEvent *evt) override;

private slots:

    void hscrollAction(int action);
    void vscrollAction(int action);

    void updateScrollbars(PatternModel::CursorChangeFlags flags);
    void setCursorFromHScroll(int value);

    void speedChanged(int value);
    void tempoChanged(int value);

    void enableAutoInstrument(bool enabled);
    void setAutoInstrument(int index);

    void updatePatternsSpin(QModelIndex const& index, int first, int last);

    void updateTempoLabel();

private:

    void paste(bool mix);

    void setTempoLabel(float tempo);
    float calcActualTempo(float speed);

    void stepDown();

    Q_DISABLE_COPY(PatternEditor)

    PianoInput const& mPianoIn;
    ModuleDocument *mDocument;

    QVBoxLayout mLayout;
        QWidget mControls;
            QVBoxLayout mControlsLayout;
                QHBoxLayout mToolbarLayout;
                    QLabel mOctaveLabel;
                    QSpinBox mOctaveSpin;
                    QLabel mEditStepLabel;
                    QSpinBox mEditStepSpin;
                    QCheckBox mLoopPatternCheck;
                    QCheckBox mFollowModeCheck;
                    QCheckBox mKeyRepeatCheck;
                QGridLayout mSettingsLayout;
                    // row 0
                    QLabel mRowsPerBeatLabel;
                    QSpinBox mRowsPerBeatSpin;
                    //
                    QLabel mSpeedLabel;
                    CustomSpinBox mSpeedSpin;
                    QLabel mSpeedActualLabel;
                    //
                    QLabel mPatternsLabel;
                    QSpinBox mPatternsSpin;
                    //
                    QComboBox mInstrumentCombo;

                    // row 1
                    QLabel mRowsPerMeasureLabel;
                    QSpinBox mRowsPerMeasureSpin;
                    //
                    QLabel mTempoLabel;
                    QSpinBox mTempoSpin;
                    QLabel mTempoActualLabel;
                    //
                    QLabel mPatternSizeLabel;
                    QSpinBox mPatternSizeSpin;
                    //
                    QCheckBox mSetInstrumentCheck;

                    std::array<QFrame, 3> mLines;
        QFrame mGridFrame;
            QGridLayout mGridLayout;
                PatternGridHeader mGridHeader;
                PatternGrid mGrid;
                QScrollBar mHScroll;
                QScrollBar mVScroll;

    QMenu mContextMenu;
    QMenu mTransposeMenu;
    Actions mActions;

    int mWheel;
    int mPageStep;

    int mPreviewKey;

    bool mSpeedLock;
    bool mScrollLock;

    PatternClipboard mClipboard;

    std::optional<uint8_t> mInstrument;

    //
    // Vector of connections for the current document, makes disconnecting
    // easy when switching documents
    //
    // make this an array later, the number of connections is static
    // a vector is used for convenience
    //
    QVector<QMetaObject::Connection> mConnections;
};
