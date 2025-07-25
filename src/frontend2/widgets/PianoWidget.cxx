
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
    , mIsKeyDown(false)
    , mNote(0)
    , mLastKeyPressed(Qt::Key_unknown)
    , mKeymap()
    , mTheme()
    , mScheme()
    , mRedrawKeys(true)
    , mWhiteKeys()
    , mBlackKeys() {
    setFocusPolicy(Qt::StrongFocus);

    setMinimumWidth(TU::cMinWidth);
    setMinimumHeight(TU::cMinHeight);
    setMaximumHeight(TU::cMaxHeight);

    calculateScheme();
}

void PianoWidget::setKeymap(NimRef<B::NoteKeymap> map) {
    mKeymap = std::move(map);
}

void PianoWidget::play(int note) {
    auto const oldnote = mNote;
    mNote = note;
    update();
    if (mIsKeyDown && oldnote != note) {
        emit keyChange(note);
    } else {
        mIsKeyDown = true;
        emit keyDown(note);
    }
}

void PianoWidget::release() {
    if (mIsKeyDown) {
        mIsKeyDown = false;
        update();
        emit keyUp();
    }
}

void PianoWidget::setColorTheme(ColorTheme const &theme) {
    mTheme = theme;
    mRedrawKeys = true;
    update();
}

void PianoWidget::focusOutEvent(QFocusEvent *evt) {
    Q_UNUSED(evt);

    if (mIsKeyDown) {
        release();
        mLastKeyPressed = Qt::Key_unknown;
    }
}

void PianoWidget::keyPressEvent(QKeyEvent *evt) {
    if (!evt->isAutoRepeat()) {
        if (mKeymap) {
            auto const key = evt->key();
            auto const note = mKeymap->keyToNote(key);
            if (note != -1) {
                mLastKeyPressed = key;
                play(note);
                return;
            }
        }
    }

    QWidget::keyPressEvent(evt);
}

void PianoWidget::keyReleaseEvent(QKeyEvent *evt) {
    if (!evt->isAutoRepeat()) {
        if (evt->key() == mLastKeyPressed) {
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
        if (!mIsKeyDown || note != mNote) {
            play(note);
        }
    } else {
        release();
    }
}

void PianoWidget::paintEvent(QPaintEvent *event) {
    (void)event;

    if (mRedrawKeys) {
        mRedrawKeys = false;
        renderPiano();
    }

    QPainter p(this);

    p.fillRect(rect(), mTheme.colors[ColorTheme::ColorDarkest]);

    auto const x = mScheme.leftPad + 1;
    p.drawPicture(x, 1, mWhiteKeys);

    if (mIsKeyDown) {
        auto const octave = mNote / 12;
        auto const noteInOctave = mNote % 12;
        auto const info = TU::cKeyInfo[noteInOctave];
        auto const whiteKeyStart =
            x + ((info.whiteKeyIndex + (octave * 7)) * mScheme.ww);
        if (info.isBlack) {
            p.drawPicture(x, 1, mBlackKeys);
            p.fillRect(whiteKeyStart + mScheme.boff, 1, mScheme.bw, mScheme.bh,
                       mTheme.colors[ColorTheme::ColorLight]);
        } else {
            p.fillRect(whiteKeyStart, 1, mScheme.ww, mScheme.wh,
                       mTheme.colors[ColorTheme::ColorLight]);
            p.drawPicture(x, 1, mBlackKeys);
        }
    } else {
        p.drawPicture(x, 1, mBlackKeys);
    }

    if (!isEnabled()) {
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.fillRect(rect(), QColor(128, 128, 128));
    }
}

void PianoWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    calculateScheme();
    mRedrawKeys = true;
}

bool PianoWidget::mouseHasNote(QPoint pos) {
    QRect rect(mScheme.leftPad, 0, mScheme.width, mScheme.wh);
    return rect.contains(pos);
}

int PianoWidget::getNoteFromMouse(QPoint mousePos) {
    auto const x = mousePos.x() - mScheme.leftPad;
    auto const y = mousePos.y() - 1;

    bool isBlack = false;
    int wkeyInOctave = x / mScheme.ww;
    int octave = wkeyInOctave / 7;
    wkeyInOctave %= 7;
    int bkeyInOctave = 0;

    if (y < mScheme.bh) {
        // check if the mouse is over a black key
        bkeyInOctave = TU::cBlackLeftOf[wkeyInOctave];
        int wkeyx = x % mScheme.ww;

        if (bkeyInOctave != TU::cKeyIndexNull && wkeyx <= (mScheme.bw / 2)) {
            // mouse is over the black key to the left of the white key
            isBlack = true;
        } else {
            // now check the right

            // get the black key to the left of the next white key
            bkeyInOctave = TU::cBlackLeftOf[(wkeyInOctave + 1) % 7];

            if (bkeyInOctave != TU::cKeyIndexNull && wkeyx >= mScheme.boff) {
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
    mScheme.ww = contentWidth / TU::cTotalWhiteKeys;
    // white key height always takes up the available height
    mScheme.wh = contentHeight;
    // black keys have 75% width and 60% height of white keys
    mScheme.bw = mScheme.ww * 3 / 4;
    mScheme.bh = mScheme.wh * 6 / 10;
    // offset from the white key left of the black key
    mScheme.boff = mScheme.ww - (mScheme.bw / 2);
    mScheme.width = mScheme.ww * TU::cTotalWhiteKeys;
    // left-pad value for centering
    mScheme.leftPad = (contentWidth - mScheme.width) / 2;
}

void PianoWidget::renderPiano() {
    // white keys
    {
        QPicture pic;
        {
            QPainter p(&pic);
            p.fillRect(0, 0, mScheme.width, mScheme.wh,
                       mTheme.colors[ColorTheme::ColorLightest]);
            p.setPen(mTheme.colors[ColorTheme::ColorDarkest]);
            int x = mScheme.ww;
            int const y = mScheme.wh - 1;
            for (int i = 0; i < TU::cTotalWhiteKeys - 1; ++i) {
                p.drawLine(x, 0, x, y);
                x += mScheme.ww;
            }
        }
        mWhiteKeys = std::move(pic);
    }

    {
        QPicture pic;
        {
            QPainter p(&pic);
            int x = mScheme.boff;
            QColor const color = mTheme.colors[ColorTheme::ColorDark];
            int const twokeys = mScheme.ww * 2;
            for (int o = 0; o < TU::cOctaves; ++o) {
                p.fillRect(x, 0, mScheme.bw, mScheme.bh, color); // C#
                x += mScheme.ww;
                p.fillRect(x, 0, mScheme.bw, mScheme.bh, color); // D#
                x += twokeys;
                p.fillRect(x, 0, mScheme.bw, mScheme.bh, color); // F#
                x += mScheme.ww;
                p.fillRect(x, 0, mScheme.bw, mScheme.bh, color); // G#
                x += mScheme.ww;
                p.fillRect(x, 0, mScheme.bw, mScheme.bh, color); // A#
                x += twokeys;
            }
        }
        mBlackKeys = std::move(pic);
    }
}

#undef TU