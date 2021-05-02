
#include "widgets/PianoWidget.hpp"

#include <QPainter>
#include <QScrollBar>

constexpr int KEYINDEX_NULL = -1;
constexpr int N_OCTAVES = 7;
constexpr int N_WHITEKEYS = 7;
constexpr int N_BLACKKEYS = 5;

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

std::optional<PianoWidget::Pixmaps> PianoWidget::mPixmaps;


PianoWidget::PianoWidget(PianoInput const& input, QWidget *parent) :
    QWidget(parent),
    mIsKeyDown(false),
    mNote(trackerboy::NOTE_C),
    mInput(input),
    mLastKeyPressed(Qt::Key_unknown)
{
    setFocusPolicy(Qt::StrongFocus);
    
    if (!mPixmaps) {
        auto &pixmaps = mPixmaps.emplace();
        pixmaps.whiteKeyDown.load(QStringLiteral(":/images/whitekey_down.png"));
        pixmaps.blackKeyDown.load(QStringLiteral(":/images/blackkey_down.png"));
        pixmaps.pianoWhitePix.load(QStringLiteral(":/images/piano_whitekeys.png"));
        pixmaps.pianoBlackPix.load(QStringLiteral(":/images/piano_blackkeys.png"));
    }
    setFixedWidth(PIANO_WIDTH);
    setFixedHeight(PIANO_HEIGHT);
}

void PianoWidget::play(trackerboy::Note note) {
    mIsKeyDown = true;
    mNote = note;
    update();
    emit keyDown(mNote);
}

void PianoWidget::release() {
    mIsKeyDown = false;
    update();
    emit keyUp();
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
        if (note) {
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
        play(getNoteFromMouse(event->x(), event->y()));
    }

}

void PianoWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        release();
    }
}

void PianoWidget::mouseMoveEvent(QMouseEvent *event) {

    int x = event->x();
    int y = event->y();
    if (rect().contains(x, y)) {
        auto note = getNoteFromMouse(x, y);
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
    KeyPaintInfo keyInfo{ 0 };

    if (mIsKeyDown) {
        octaveOffset = mNote / 12;
        int keyInOctave = mNote % 12;
        octaveOffset *= N_WHITEKEYS * WKEY_WIDTH;

        keyInfo = KEY_INFO[keyInOctave];
    }

    auto &pixmaps = mPixmaps.value();

    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmaps.pianoWhitePix);
    
    if (mIsKeyDown && !keyInfo.isBlack) {
        painter.drawPixmap(octaveOffset + keyInfo.xoffset, 0, pixmaps.whiteKeyDown);
    }

    painter.drawPixmap(0, 0, pixmaps.pianoBlackPix);

    if (mIsKeyDown && keyInfo.isBlack) {
        painter.drawPixmap(octaveOffset + keyInfo.xoffset, 0, pixmaps.blackKeyDown);
    }

}



trackerboy::Note PianoWidget::getNoteFromMouse(int x, int y) {
    bool isBlack = false;
    int wkeyInOctave = x / WKEY_WIDTH;
    int octave = wkeyInOctave / N_WHITEKEYS;
    wkeyInOctave %= N_WHITEKEYS;
    int bkeyInOctave = 0;

    if (y < BKEY_HEIGHT) {
        // check if the mouse is over a black key
        bkeyInOctave = BLACKKEY_LEFTOF[wkeyInOctave];
        int wkeyx = x % WKEY_WIDTH;

        if (bkeyInOctave != KEYINDEX_NULL && wkeyx <= BKEY_WIDTH_HALF) {
            // mouse is over the black key to the left of the white key
            isBlack = true;
        } else {
            // now check the right
            
            // get the black key to the left of the next white key
            bkeyInOctave = BLACKKEY_LEFTOF[wkeyInOctave == N_WHITEKEYS - 1 ? 0 : wkeyInOctave + 1];
            
            if (bkeyInOctave != KEYINDEX_NULL && wkeyx >= WKEY_WIDTH - BKEY_WIDTH_HALF) {
                isBlack = true;
            }
        }
    }

    int note = octave * 12;
    if (isBlack) {
        note += BLACKKEY_TO_NOTE[bkeyInOctave];
    } else {
        note += WHITEKEY_TO_NOTE[wkeyInOctave];
    }

    return static_cast<trackerboy::Note>(note);
}

