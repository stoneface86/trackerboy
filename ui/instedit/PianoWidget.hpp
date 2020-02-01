
#pragma once

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>

#include "trackerboy/note.hpp"

namespace instedit {


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
    QPixmap whiteKeyDown;
    QPixmap whiteKeyUp;
    QPixmap blackKeyDown;
    QPixmap blackKeyUp;
    bool isKeyDown;
    int bkeyIndex; // index of the black key being played
    int wkeyIndex; // index of the white key being played

    // offsets from an octave to draw a black key
    // 0 = C#, ..., 4 = A#
    int blackKeyOffsets[5];

    bool updateKeySelection(QMouseEvent *event);
    trackerboy::Note note();

};



}
