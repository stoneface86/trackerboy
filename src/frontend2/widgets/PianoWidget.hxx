
#pragma once

#include "backend.hxx"

#include <QWidget>

class PianoWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PianoWidget(QWidget *parent = nullptr);
    virtual ~PianoWidget() = default;

    void setKeymap(NimRef<B::NoteKeymap> map);

    void play(int note);
    void release();

signals:
    void keyDown(int note);
    void keyChange(int note);
    void keyUp();

protected:
    virtual void focusOutEvent(QFocusEvent *evt) override;
    virtual void keyPressEvent(QKeyEvent *evt) override;
    virtual void keyReleaseEvent(QKeyEvent *evt) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

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
    
    int getNoteFromMouse(QPoint mousePos);

    bool mIsKeyDown;
    int mNote;

    int mLastKeyPressed;
    NimRef<B::NoteKeymap> mKeymap;
};
