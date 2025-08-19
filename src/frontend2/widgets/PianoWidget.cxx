
#include "widgets/PianoWidget.hxx"
#include "utils/aliases.hxx"

#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>

#include <array>

#define TU PianoWidgetTU
namespace TU {

constexpr int cKeyIndexNull = -1;
constexpr int cOctaves = 7;
constexpr int cTotalWhiteKeys = cOctaves * 7;

constexpr i8 cKeyMinWidth = 12;
constexpr i8 cKeyMinHeight = 62;
constexpr i16 cMinWidth = 7 * cOctaves * cKeyMinWidth + 2;
constexpr i16 cMinHeight = cKeyMinHeight + 2;
constexpr i16 cMaxHeight = 128;

// white key index: 0..6 ==> C, D, E, F, G, A, B
// black key index: 0..4 ==> C#, D#, F#, G#, A#

// lookup table gets the black key to the left of the given white key index
// for the right of a white key, increment the index by 1
static std::array<i8, 7> const cBlackLeftOf = {
    cKeyIndexNull, // C -> none
    0,             // D -> C#
    1,             // E -> D#
    cKeyIndexNull, // F -> none
    2,             // G -> F#
    3,             // A -> G#
    4,             // B -> A#
};

struct KeyPaintInfo {
    bool isBlack;
    i8 whiteKeyIndex;
};

static std::array<KeyPaintInfo, 12> const cKeyInfo = {{
    {false, 0}, // C
    {true, 0},  // C#
    {false, 1}, // D
    {true, 1},  // D#
    {false, 2}, // E
    {false, 3}, // F
    {true, 3},  // F#
    {false, 4}, // G
    {true, 4},  // G#
    {false, 5}, // A
    {true, 5},  // A#
    {false, 6}  // B
}};

// table to convert a white key index to a trackerboy note
static std::array<i8, 7> const cWhiteToNote = {
    B::NoteC, B::NoteD, B::NoteE, B::NoteF, B::NoteG, B::NoteA, B::NoteB};

// table converts a black key index to a trackerboy note
static std::array<i8, 5> const cBlackToNote = {
    B::NoteDb, // C#
    B::NoteEb, // D#
    B::NoteGb, // F#
    B::NoteAb, // G#
    B::NoteBb  // A#
};

} // namespace TU

PianoWidget::PianoWidget(QWidget *parent)
    : QWidget(parent)
    , _isKeyDown(false)
    , _note(0)
    , _lastKeyPressed(Qt::Key_unknown)
    , _keymap()
    , _theme()
    , _scheme()
    , _redrawKeys(true)
    , _whiteKeys()
    , _blackKeys() {
    setFocusPolicy(Qt::StrongFocus);

    setMinimumWidth(TU::cMinWidth);
    setMinimumHeight(TU::cMinHeight);
    setMaximumHeight(TU::cMaxHeight);

    calculateScheme();
}

void PianoWidget::setKeymap(NimRef<B::NoteKeymap> map) {
    _keymap = std::move(map);
}

void PianoWidget::play(int note) {
    auto const oldnote = _note;
    _note = note;
    update();
    if (_isKeyDown && oldnote != note) {
        emit keyChange(note);
    } else {
        _isKeyDown = true;
        emit keyDown(note);
    }
}

void PianoWidget::release() {
    if (_isKeyDown) {
        _isKeyDown = false;
        update();
        emit keyUp();
    }
}

void PianoWidget::setColorTheme(ColorTheme const &theme) {
    _theme = theme;
    _redrawKeys = true;
    update();
}

void PianoWidget::focusOutEvent(QFocusEvent *evt) {
    Q_UNUSED(evt);

    if (_isKeyDown) {
        release();
        _lastKeyPressed = Qt::Key_unknown;
    }
}

void PianoWidget::keyPressEvent(QKeyEvent *evt) {
    if (!evt->isAutoRepeat()) {
        if (_keymap) {
            auto const key = evt->key();
            auto const note = _keymap->keyToNote(key);
            if (note != -1) {
                _lastKeyPressed = key;
                play(note);
                return;
            }
        }
    }

    QWidget::keyPressEvent(evt);
}

void PianoWidget::keyReleaseEvent(QKeyEvent *evt) {
    if (!evt->isAutoRepeat()) {
        if (evt->key() == _lastKeyPressed) {
            release();
            return;
        }
    }

    QWidget::keyReleaseEvent(evt);
}

void PianoWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        auto const pos = event->position().toPoint();
        if (mouseHasNote(pos)) {
            play(getNoteFromMouse(pos));
        }
    }
}

void PianoWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        release();
    }
}

void PianoWidget::mouseMoveEvent(QMouseEvent *event) {

    if (!event->buttons().testFlag(Qt::LeftButton)) {
        return;
    }

    auto const pos = event->position().toPoint();
    if (mouseHasNote(pos)) {
        auto const note = getNoteFromMouse(pos);
        if (!_isKeyDown || note != _note) {
            play(note);
        }
    } else {
        release();
    }
}

void PianoWidget::paintEvent(QPaintEvent *event) {
    (void)event;

    if (_redrawKeys) {
        _redrawKeys = false;
        renderPiano();
    }

    QPainter p(this);

    p.fillRect(rect(), _theme.colors[ColorTheme::ColorDarkest]);

    auto const x = _scheme.leftPad + 1;
    p.drawPicture(x, 1, _whiteKeys);

    if (_isKeyDown) {
        auto const octave = _note / 12;
        auto const noteInOctave = _note % 12;
        auto const info = TU::cKeyInfo[noteInOctave];
        auto const whiteKeyStart =
            x + ((info.whiteKeyIndex + (octave * 7)) * _scheme.ww);
        if (info.isBlack) {
            p.drawPicture(x, 1, _blackKeys);
            p.fillRect(whiteKeyStart + _scheme.boff, 1, _scheme.bw, _scheme.bh,
                       _theme.colors[ColorTheme::ColorLight]);
        } else {
            p.fillRect(whiteKeyStart, 1, _scheme.ww, _scheme.wh,
                       _theme.colors[ColorTheme::ColorLight]);
            p.drawPicture(x, 1, _blackKeys);
        }
    } else {
        p.drawPicture(x, 1, _blackKeys);
    }

    if (!isEnabled()) {
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.fillRect(rect(), QColor(128, 128, 128));
    }
}

void PianoWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    calculateScheme();
    _redrawKeys = true;
}

bool PianoWidget::mouseHasNote(QPoint pos) {
    QRect rect(_scheme.leftPad, 0, _scheme.width, _scheme.wh);
    return rect.contains(pos);
}

int PianoWidget::getNoteFromMouse(QPoint mousePos) {
    auto const x = mousePos.x() - _scheme.leftPad;
    auto const y = mousePos.y() - 1;

    bool isBlack = false;
    int wkeyInOctave = x / _scheme.ww;
    int octave = wkeyInOctave / 7;
    wkeyInOctave %= 7;
    int bkeyInOctave = 0;

    if (y < _scheme.bh) {
        // check if the mouse is over a black key
        bkeyInOctave = TU::cBlackLeftOf[wkeyInOctave];
        int wkeyx = x % _scheme.ww;

        if (bkeyInOctave != TU::cKeyIndexNull && wkeyx <= (_scheme.bw / 2)) {
            // mouse is over the black key to the left of the white key
            isBlack = true;
        } else {
            // now check the right

            // get the black key to the left of the next white key
            bkeyInOctave = TU::cBlackLeftOf[(wkeyInOctave + 1) % 7];

            if (bkeyInOctave != TU::cKeyIndexNull && wkeyx >= _scheme.boff) {
                isBlack = true;
            }
        }
    }

    int note = octave * 12;
    if (isBlack) {
        note += TU::cBlackToNote[bkeyInOctave];
    } else {
        note += TU::cWhiteToNote[wkeyInOctave];
    }

    return note;
}

void PianoWidget::calculateScheme() {
    auto const sz = size();
    auto const contentWidth = sz.width() - 2;
    auto const contentHeight = sz.height() - 2;
    // divide total available width by the total number of white keys
    _scheme.ww = contentWidth / TU::cTotalWhiteKeys;
    // white key height always takes up the available height
    _scheme.wh = contentHeight;
    // black keys have 75% width and 60% height of white keys
    _scheme.bw = _scheme.ww * 3 / 4;
    _scheme.bh = _scheme.wh * 6 / 10;
    // offset from the white key left of the black key
    _scheme.boff = _scheme.ww - (_scheme.bw / 2);
    _scheme.width = _scheme.ww * TU::cTotalWhiteKeys;
    // left-pad value for centering
    _scheme.leftPad = (contentWidth - _scheme.width) / 2;
}

void PianoWidget::renderPiano() {
    // white keys
    {
        QPicture pic;
        {
            QPainter p(&pic);
            p.fillRect(0, 0, _scheme.width, _scheme.wh,
                       _theme.colors[ColorTheme::ColorLightest]);
            p.setPen(_theme.colors[ColorTheme::ColorDarkest]);
            int x = _scheme.ww;
            int const y = _scheme.wh - 1;
            for (int i = 0; i < TU::cTotalWhiteKeys - 1; ++i) {
                p.drawLine(x, 0, x, y);
                x += _scheme.ww;
            }
        }
        _whiteKeys = std::move(pic);
    }

    {
        QPicture pic;
        {
            QPainter p(&pic);
            int x = _scheme.boff;
            QColor const color = _theme.colors[ColorTheme::ColorDark];
            int const twokeys = _scheme.ww * 2;
            for (int o = 0; o < TU::cOctaves; ++o) {
                p.fillRect(x, 0, _scheme.bw, _scheme.bh, color); // C#
                x += _scheme.ww;
                p.fillRect(x, 0, _scheme.bw, _scheme.bh, color); // D#
                x += twokeys;
                p.fillRect(x, 0, _scheme.bw, _scheme.bh, color); // F#
                x += _scheme.ww;
                p.fillRect(x, 0, _scheme.bw, _scheme.bh, color); // G#
                x += _scheme.ww;
                p.fillRect(x, 0, _scheme.bw, _scheme.bh, color); // A#
                x += twokeys;
            }
        }
        _blackKeys = std::move(pic);
    }
}

#undef TU