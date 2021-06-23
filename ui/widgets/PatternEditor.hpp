
#pragma once

#include "core/clipboard/PatternClipboard.hpp"
#include "core/PianoInput.hpp"
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/PatternGridHeader.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QScrollBar>

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

    void previewNote(int note, int track, int instrument);

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

private:

    void pasteImpl(bool mix);

    void setTempoLabel(float tempo);
    float calcActualTempo(float speed);

    void stepDown();

    Q_DISABLE_COPY(PatternEditor)

    PianoInput const& mPianoIn;
    ModuleDocument *mDocument;

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

    std::array<QMetaObject::Connection, 4> mConnections;


};
