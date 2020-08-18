
#pragma once

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>

#include "trackerboy/note.hpp"

namespace instedit {

//
// Custom widget for a piano control. This control displays a piano keyboard with an
// 8 octaves. Individual keys can be pressed using the mouse.
//
class PianoWidget : public QWidget {

    Q_OBJECT

public:
    PianoWidget(QWidget *parent = nullptr);

signals:
    void keyDown(trackerboy::Note note);
    void keyUp();


protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPixmap mWhiteKeyDown;
    QPixmap mBlackKeyDown;
    QPixmap mPianoWhitePix;
    QPixmap mPianoBlackPix;

    bool mIsKeyDown;
    trackerboy::Note mNote;

    void setNoteFromMouse(int x, int y);


};



}
