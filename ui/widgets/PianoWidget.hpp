
#pragma once

#include "core/PianoInput.hpp"

#include "trackerboy/note.hpp"

#include <QWidget>
#include <QPixmap>
#include <QPixmapCache>
#include <QMouseEvent>

#include <optional>

//
// Custom widget for a piano control. This control displays a piano keyboard with an
// 8 octaves. Individual keys can be pressed using the mouse.
//
class PianoWidget : public QWidget {

    Q_OBJECT

public:
    PianoWidget(PianoInput &input, QWidget *parent = nullptr);

    void play(trackerboy::Note note);
    void release();

signals:
    void keyDown(trackerboy::Note note);
    void keyUp();


protected:
    void focusOutEvent(QFocusEvent *evt) override;
    void keyPressEvent(QKeyEvent *evt) override;
    void keyReleaseEvent(QKeyEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    
    trackerboy::Note getNoteFromMouse(int x, int y);
    
    struct Pixmaps {
        QPixmap whiteKeyDown;
        QPixmap blackKeyDown;
        QPixmap pianoWhitePix;
        QPixmap pianoBlackPix;
    };

    // pixmaps are stored statically, so that each PianoWidget doesn't have
    // to load in its own copy.
    static std::optional<Pixmaps> mPixmaps;

    bool mIsKeyDown;
    trackerboy::Note mNote;

    PianoInput &mInput;
    int mLastKeyPressed;


};



