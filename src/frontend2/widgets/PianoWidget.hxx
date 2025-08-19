
#pragma once

#include "backend.hxx"
#include "core/ColorTheme.hxx"
#include "utils/aliases.hxx"

#include <QPicture>
#include <QWidget>

class PianoWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PianoWidget(QWidget *parent = nullptr);

    void setKeymap(NimRef<B::NoteKeymap> map);

    void play(int note);
    void release();
    void setColorTheme(ColorTheme const &theme);

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
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(PianoWidget)

    struct Scheme {
        i16 ww;   // white key width
        i16 wh;   // white key height
        i16 bw;   // black key width
        i16 bh;   // black key height
        i16 boff; // black key offset from white key
        i16 width;
        i16 leftPad;
    };

    bool mouseHasNote(QPoint mousePos);

    int getNoteFromMouse(QPoint mousePos);

    void renderPiano();

    void calculateScheme();

    bool _isKeyDown;
    int _note;

    int _lastKeyPressed;
    NimRef<B::NoteKeymap> _keymap;

    ColorTheme _theme;
    Scheme _scheme;
    bool _redrawKeys;
    QPicture _whiteKeys;
    QPicture _blackKeys;
};
