
#include "widgets/PianoWidget.hpp"

#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>

#include <QtDebug>

#define TU PianoWidgetTU
namespace TU {

constexpr int KEYINDEX_NULL = -1;
constexpr int N_OCTAVES = 7;
constexpr int N_WHITEKEYS = 7;

// IMPORTANT! these widths must match the widths of the key images

constexpr int WKEY_WIDTH = 12;
constexpr int BKEY_WIDTH = 8;
constexpr int BKEY_HEIGHT = 42;
constexpr int PIANO_WIDTH = N_OCTAVES * N_WHITEKEYS * WKEY_WIDTH;
constexpr int PIANO_HEIGHT = 64;

constexpr int BKEY_WIDTH_HALF = BKEY_WIDTH / 2;

// white key index: 0..6 ==> C, D, E, F, G, A, B
// black key index: 0..4 ==> C#, D#, F#, G#, A#


// lookup table gets the black key to the left of the given white key index
// for the right of a white key, increment the index by 1
static const int BLACKKEY_LEFTOF[] = {
    KEYINDEX_NULL,  // C -> none
    0,              // D -> C#
    1,              // E -> D#
    KEYINDEX_NULL,  // F -> none
    2,              // G -> F#
    3,              // A -> G#
    4,              // B -> A#
};

struct KeyPaintInfo {
    bool isBlack;
    int xoffset;
};

static const KeyPaintInfo KEY_INFO[] = {
    { false,    0 },                                    // C
    { true,     WKEY_WIDTH - BKEY_WIDTH_HALF },         // C#
    { false,    WKEY_WIDTH * 1 },                       // D
    { true,     WKEY_WIDTH * 2 - BKEY_WIDTH_HALF },     // D#
    { false,    WKEY_WIDTH * 2 },                       // E
    { false,    WKEY_WIDTH * 3 },                       // F
    { true,     WKEY_WIDTH * 4 - BKEY_WIDTH_HALF },     // F#
    { false,    WKEY_WIDTH * 4 },                       // G
    { true,     WKEY_WIDTH * 5 - BKEY_WIDTH_HALF },     // G#
    { false,    WKEY_WIDTH * 5 },                       // A
    { true,     WKEY_WIDTH * 6 - BKEY_WIDTH_HALF },     // A#
    { false,    WKEY_WIDTH * 6 },                       // B
};

// table to convert a white key index to a trackerboy note
static const trackerboy::Note WHITEKEY_TO_NOTE[] = {
    trackerboy::NOTE_C,
    trackerboy::NOTE_D,
    trackerboy::NOTE_E,
    trackerboy::NOTE_F,
    trackerboy::NOTE_G,
    trackerboy::NOTE_A,
    trackerboy::NOTE_B
};

// table converts a black key index to a trackerboy note
static const trackerboy::Note BLACKKEY_TO_NOTE[] = {
    trackerboy::NOTE_Db, // C#
    trackerboy::NOTE_Eb, // D#
    trackerboy::NOTE_Gb, // F#
    trackerboy::NOTE_Ab, // G#
    trackerboy::NOTE_Bb  // A#
};

static std::array const PIXMAP_PATHS = {
    ":/images/whitekey_down.png",
    ":/images/blackkey_down.png",
    ":/images/piano_whitekeys.png",
    ":/images/piano_blackkeys.png"
};

}



PianoWidget::PianoWidget(PianoInput const& input, QWidget *parent) :
    QWidget(parent),
    mIsKeyDown(false),
    mNote(trackerboy::NOTE_C),
    mInput(input),
    mLastKeyPressed(Qt::Key_unknown)
{
    setFocusPolicy(Qt::StrongFocus);
    
    setFixedWidth(TU::PIANO_WIDTH);
    setFixedHeight(TU::PIANO_HEIGHT);
}

void PianoWidget::play(int note) {
    mNote = note;
    update();
    if (mIsKeyDown) {
        emit keyChange(mNote);
    } else {
        mIsKeyDown = true;
        emit keyDown(mNote);
    }
}

void PianoWidget::release() {
    mIsKeyDown = false;
    update();
    emit keyUp();
}

void PianoWidget::midiNoteOn(int note) {
    if (isEnabled()) {
        play(note);
    }
}

void PianoWidget::midiNoteOff() {
    if (isEnabled()) {
        release();
    }
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
        auto key = evt->key();
        auto note = mInput.keyToNote(key);
        if (note && *note <= trackerboy::NOTE_LAST) {
            mLastKeyPressed = key;
            play(note.value());
            return;
        }
    }

    QWidget::keyPressEvent(evt);
}

void PianoWidget::keyReleaseEvent(QKeyEvent *evt) {
    if (!evt->isAutoRepeat()) {
        auto key = evt->key();
        auto note = mInput.keyToNote(key);
        if (note && key == mLastKeyPressed) {
            release();
            return;
        }
    }

    QWidget::keyReleaseEvent(evt);
}

void PianoWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        play(getNoteFromMouse(event->position().toPoint()));
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
    if (rect().contains(pos)) {
        auto note = getNoteFromMouse(pos);
        if (!mIsKeyDown || note != mNote) {
            play(note);
        }
    } else {
        release();
    }

}

void PianoWidget::paintEvent(QPaintEvent *event) {
    (void)event;

    int octaveOffset = 0;
    TU::KeyPaintInfo keyInfo{ false, 0 };

    if (mIsKeyDown) {
        octaveOffset = mNote / 12;
        int keyInOctave = mNote % 12;
        octaveOffset *= TU::N_WHITEKEYS * TU::WKEY_WIDTH;

        keyInfo = TU::KEY_INFO[keyInOctave];
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, getPixmap(PixWhiteKeys));
    
    if (mIsKeyDown && !keyInfo.isBlack) {
        painter.drawPixmap(octaveOffset + keyInfo.xoffset, 0, getPixmap(PixWhiteKeyDown));
    }

    painter.drawPixmap(0, 0, getPixmap(PixBlackKeys));

    if (mIsKeyDown && keyInfo.isBlack) {
        painter.drawPixmap(octaveOffset + keyInfo.xoffset, 0, getPixmap(PixBlackKeyDown));
    }

    if (!isEnabled()) {
        painter.setCompositionMode(QPainter::CompositionMode_Plus);
        painter.fillRect(rect(), QColor(128, 128, 128));
    }

}



int PianoWidget::getNoteFromMouse(QPoint mousePos) {
    auto const x = mousePos.x();
    auto const y = mousePos.y();

    bool isBlack = false;
    int wkeyInOctave = x / TU::WKEY_WIDTH;
    int octave = wkeyInOctave / TU::N_WHITEKEYS;
    wkeyInOctave %= TU::N_WHITEKEYS;
    int bkeyInOctave = 0;

    if (y < TU::BKEY_HEIGHT) {
        // check if the mouse is over a black key
        bkeyInOctave = TU::BLACKKEY_LEFTOF[wkeyInOctave];
        int wkeyx = x % TU::WKEY_WIDTH;

        if (bkeyInOctave != TU::KEYINDEX_NULL && wkeyx <= TU::BKEY_WIDTH_HALF) {
            // mouse is over the black key to the left of the white key
            isBlack = true;
        } else {
            // now check the right
            
            // get the black key to the left of the next white key
            bkeyInOctave = TU::BLACKKEY_LEFTOF[wkeyInOctave == TU::N_WHITEKEYS - 1 ? 0 : wkeyInOctave + 1];
            
            if (bkeyInOctave != TU::KEYINDEX_NULL && wkeyx >= TU::WKEY_WIDTH - TU::BKEY_WIDTH_HALF) {
                isBlack = true;
            }
        }
    }

    int note = octave * 12;
    if (isBlack) {
        note += TU::BLACKKEY_TO_NOTE[bkeyInOctave];
    } else {
        note += TU::WHITEKEY_TO_NOTE[wkeyInOctave];
    }

    return note;
}

QPixmap PianoWidget::getPixmap(Pixmaps id) {
    QPixmap pixmap;
    QString const key = TU::PIXMAP_PATHS[id];
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.load(key);
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}

