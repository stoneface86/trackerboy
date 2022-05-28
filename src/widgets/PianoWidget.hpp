
#pragma once

#include "midi/IMidiReceiver.hpp"
#include "config/data/PianoInput.hpp"
#include "verdigris/wobjectdefs.h"

#include <QWidget>
#include <QPixmap>

#include <array>
#include <optional>

//
// Custom widget for a piano control. This control displays a piano keyboard with an
// 8 octaves. Individual keys can be pressed using the mouse.
//
class PianoWidget : public QWidget, public IMidiReceiver {

    W_OBJECT(PianoWidget)

public:
    PianoWidget(PianoInput const& input, QWidget *parent = nullptr);
    virtual ~PianoWidget() = default;

    void play(int note);
    void release();

    virtual void midiNoteOn(int note) override;

    virtual void midiNoteOff() override;

//signals:
    void keyDown(int note) W_SIGNAL(keyDown, note)
    void keyChange(int note) W_SIGNAL(keyChange, note)
    void keyUp() W_SIGNAL(keyUp)


protected:
    void focusOutEvent(QFocusEvent *evt) override;
    void keyPressEvent(QKeyEvent *evt) override;
    void keyReleaseEvent(QKeyEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    Q_DISABLE_COPY(PianoWidget)

    enum Pixmaps {
        PixWhiteKeyDown,
        PixBlackKeyDown,
        PixWhiteKeys,
        PixBlackKeys,

        PixCount
    };

    QPixmap getPixmap(Pixmaps id);
    
    int getNoteFromMouse(int x, int y);

    bool mIsKeyDown;
    int mNote;

    PianoInput const& mInput;
    int mLastKeyPressed;


};



