
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

    explicit PatternEditor(PianoInput const& input, QWidget *parent = nullptr);
    ~PatternEditor() = default;

    PatternGrid& grid();

    void setColors(ColorTable const& colors);

    virtual bool event(QEvent *evt) override;

signals:

    void changeOctave(int octave);

    void nextInstrument();

    void previousInstrument();

    void previewNote(quint8 note);

    void stopNotePreview();

public slots:

    void setDocument(ModuleDocument *doc);

    void setInstrument(int index);

    void cut();

    void copy();

    void paste();

    void pasteMix();

    void erase();

    void selectAll();

    void increaseNote();

    void decreaseNote();

    void increaseOctave();

    void decreaseOctave();

    void transpose();

    void reverse();

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

    void pasteImpl(bool mix);

    void setTempoLabel(float tempo);
    float calcActualTempo(float speed);

    void stepDown();

    Q_DISABLE_COPY(PatternEditor)

    PianoInput const& mPianoIn;
    ModuleDocument *mDocument;

    // QVBoxLayout mLayout;
    //     QWidget mControls;
    //         QVBoxLayout mControlsLayout;
    //             QGridLayout mSettingsLayout;
    //                 // row 0
    //                 QLabel mRowsPerBeatLabel;
    //                 QSpinBox mRowsPerBeatSpin;
    //                 //
    //                 QLabel mSpeedLabel;
    //                 CustomSpinBox mSpeedSpin;
    //                 QLabel mSpeedActualLabel;
    //                 //
    //                 QLabel mPatternsLabel;
    //                 QSpinBox mPatternsSpin;
    //                 //
                    

    //                 // row 1
    //                 QLabel mRowsPerMeasureLabel;
    //                 QSpinBox mRowsPerMeasureSpin;
    //                 //
    //                 QLabel mTempoLabel;
    //                 QSpinBox mTempoSpin;
    //                 QLabel mTempoActualLabel;
    //                 //
    //                 QLabel mPatternSizeLabel;
    //                 QSpinBox mPatternSizeSpin;
    //                 //
                    

    //                 std::array<QFrame, 2> mLines;
    //     QFrame mGridFrame;
    QGridLayout mLayout;
        PatternGridHeader mGridHeader;
        PatternGrid mGrid;
        QScrollBar mHScroll;
        QScrollBar mVScroll;

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
