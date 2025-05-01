
#include "utils/aliases.hxx"
#include "widgets/PianoWidget.hxx"

#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>

#include <array>

#define TU PianoWidgetTU
namespace TU {

constexpr int cKeyIndexNull = -1;
constexpr int cOctaves = 7;
constexpr int cWhiteKeys = 7;

// IMPORTANT! these widths must match the widths of the key images

constexpr i16 cWhiteWidth = 12;
constexpr i16 cBlackWidth = 8;
constexpr i16 cBlackHeight = 42;
constexpr i16 cPianoWidth = cOctaves * cWhiteKeys * cWhiteWidth;
constexpr i16 cPianoHeight = 64;
constexpr i16 cBlackWidthHalf = cBlackWidth / 2;

// white key index: 0..6 ==> C, D, E, F, G, A, B
// black key index: 0..4 ==> C#, D#, F#, G#, A#

// lookup table gets the black key to the left of the given white key index
// for the right of a white key, increment the index by 1
static std::array<i8, 7> const cBlackLeftOf = {
    cKeyIndexNull,  // C -> none
    0,              // D -> C#
    1,              // E -> D#
    cKeyIndexNull,  // F -> none
    2,              // G -> F#
    3,              // A -> G#
    4,              // B -> A#
};

struct KeyPaintInfo {
    bool isBlack;
    i16 xoffset;
};

static std::array<KeyPaintInfo, 12> const cKeyInfo = {{
    { false,    0 },                                     // C
    { true,     cWhiteWidth - cBlackWidthHalf },         // C#
    { false,    cWhiteWidth * 1 },                       // D
    { true,     cWhiteWidth * 2 - cBlackWidthHalf },     // D#
    { false,    cWhiteWidth * 2 },                       // E
    { false,    cWhiteWidth * 3 },                       // F
    { true,     cWhiteWidth * 4 - cBlackWidthHalf },     // F#
    { false,    cWhiteWidth * 4 },                       // G
    { true,     cWhiteWidth * 5 - cBlackWidthHalf },     // G#
    { false,    cWhiteWidth * 5 },                       // A
    { true,     cWhiteWidth * 6 - cBlackWidthHalf },     // A#
    { false,    cWhiteWidth * 6 }                        // B
}};

// table to convert a white key index to a trackerboy note
static std::array<i8, 7> const cWhiteToNote = {
    B::NoteC,
    B::NoteD,
    B::NoteE,
    B::NoteF,
    B::NoteG,
    B::NoteA,
    B::NoteB
};

// table converts a black key index to a trackerboy note
static std::array<i8, 5> const cBlackToNote = {
    B::NoteDb, // C#
    B::NoteEb, // D#
    B::NoteGb, // F#
    B::NoteAb, // G#
    B::NoteBb  // A#
};

} // TU

PianoWidget::PianoWidget(QWidget *parent) :
    QWidget(parent),
    mIsKeyDown(false),
    mNote(0),
    mLastKeyPressed(Qt::Key_unknown),
    mKeymap()
{
    setFocusPolicy(Qt::StrongFocus);
    
    setFixedWidth(TU::cPianoWidth);
    setFixedHeight(TU::cPianoHeight);
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

    int octaveOffset = 0;
    TU::KeyPaintInfo keyInfo{ false, 0 };

    if (mIsKeyDown) {
        octaveOffset = mNote / 12;
        int keyInOctave = mNote % 12;
        octaveOffset *= TU::cWhiteKeys * TU::cWhiteWidth;

        keyInfo = TU::cKeyInfo[keyInOctave];
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
    int wkeyInOctave = x / TU::cWhiteWidth;
    int octave = wkeyInOctave / TU::cWhiteKeys;
    wkeyInOctave %= TU::cWhiteKeys;
    int bkeyInOctave = 0;

    if (y < TU::cBlackHeight) {
        // check if the mouse is over a black key
        bkeyInOctave = TU::cBlackLeftOf[wkeyInOctave];
        int wkeyx = x % TU::cWhiteWidth;

        if (bkeyInOctave != TU::cKeyIndexNull && wkeyx <= TU::cBlackWidthHalf) {
            // mouse is over the black key to the left of the white key
            isBlack = true;
        } else {
            // now check the right
            
            // get the black key to the left of the next white key
            bkeyInOctave = TU::cBlackLeftOf[wkeyInOctave == TU::cWhiteKeys - 1 ? 0 : wkeyInOctave + 1];
            
            if (bkeyInOctave != TU::cKeyIndexNull && wkeyx >= TU::cWhiteWidth - TU::cBlackWidthHalf) {
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

QPixmap PianoWidget::getPixmap(Pixmaps id) {
    static std::array<const char*, PixCount> const cPaths = {
        ":/images/whitekey-down.png",
        ":/images/blackkey-down.png",
        ":/images/whitekey-all.png",
        ":/images/blackkey-all.png"
    };

    QPixmap pixmap;
    QString const key = cPaths[id];
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap.load(key);
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}




#undef TU