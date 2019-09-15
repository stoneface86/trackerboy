
#include "PianoWidget.hpp"

#include <QPainter>

constexpr int KEYINDEX_NULL = -1;
constexpr int N_OCTAVES = 7;
constexpr int N_WHITEKEYS = 7;
constexpr int N_BLACKKEYS = 5;

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

static const int WHITEKEY_TO_NOTE[] = {
    trackerboy::NOTE_C,
    trackerboy::NOTE_D,
    trackerboy::NOTE_E,
    trackerboy::NOTE_F,
    trackerboy::NOTE_G,
    trackerboy::NOTE_A,
    trackerboy::NOTE_B
};

static const int BLACKKEY_TO_NOTE[] = {
    trackerboy::NOTE_Db, // C#
    trackerboy::NOTE_Eb, // D#
    trackerboy::NOTE_Gb, // F#
    trackerboy::NOTE_Ab, // G#
    trackerboy::NOTE_Bb  // A#
};


namespace instedit {


PianoWidget::PianoWidget(QWidget *parent) :
    whiteKeyDown(":/images/whitekey_down.png"),
    whiteKeyUp(":/images/whitekey_up.png"),
    blackKeyDown(":/images/blackkey_down.png"),
    blackKeyUp(":/images/blackkey_up.png"),
    isKeyDown(false),
    bkeyIndex(KEYINDEX_NULL),
    wkeyIndex(KEYINDEX_NULL),
    QWidget(parent)
{
    int wkwidth = whiteKeyUp.width();
    int bkhalf = blackKeyUp.width() / 2;

    setFixedWidth(wkwidth * N_WHITEKEYS * N_OCTAVES);
    setFixedHeight(whiteKeyUp.height());
    setMouseTracking(true);

    blackKeyOffsets[0] = wkwidth - bkhalf;       // C#
    blackKeyOffsets[1] = (wkwidth * 2) - bkhalf; // D#
    blackKeyOffsets[2] = (wkwidth * 4) - bkhalf; // F#
    blackKeyOffsets[3] = (wkwidth * 5) - bkhalf; // G#
    blackKeyOffsets[4] = (wkwidth * 6) - bkhalf; // A#

}


void PianoWidget::paintEvent(QPaintEvent *event) {

    QPainter painter(this);
    painter.drawTiledPixmap(0, 0, width(), height(), whiteKeyUp);
    if (isKeyDown && wkeyIndex != KEYINDEX_NULL) {
        painter.drawPixmap(wkeyIndex * whiteKeyDown.width(), 0, whiteKeyDown);
    }

    // black keys
    int x = 0;
    int octaveWidth = whiteKeyUp.width() * N_OCTAVES;
    int index = 0;
    for (int i = 0; i != N_OCTAVES; ++i) {
        for (int j = 0; j != sizeof(blackKeyOffsets) / sizeof(int); ++j) {
            painter.drawPixmap(
                x + blackKeyOffsets[j],
                0,
                (bkeyIndex == index && isKeyDown) ? blackKeyDown : blackKeyUp
            );

            ++index;
        }
        x += octaveWidth;
    }


}

void PianoWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isKeyDown = true;
        updateKeySelection(event);
        repaint();
        emit keyDown(note());

        
    }
    
}

void PianoWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isKeyDown = false;
        updateKeySelection(event);
        repaint();
        emit keyUp();
    }
}

void PianoWidget::mouseMoveEvent(QMouseEvent *event) {

    if (isKeyDown) {
        
        if (updateKeySelection(event)) {
            repaint();
            emit keyDown(note());
        }

    }
}

bool PianoWidget::updateKeySelection(QMouseEvent *event) {
    int x = event->x();
    int y = event->y();

    int wkwidth = whiteKeyUp.width();
    int bkhalf = blackKeyUp.width() / 2;


    int wkeyIndexNew = KEYINDEX_NULL;
    int bkeyIndexNew = KEYINDEX_NULL;

    if (rect().contains(x, y)) {
        wkeyIndexNew = x / wkwidth;
        // check if we hit a black key
        if (y < blackKeyUp.height()) {
            int wkey = wkeyIndexNew % N_WHITEKEYS;
            int octave = wkeyIndexNew / N_WHITEKEYS;
            int wkeyx = x % wkwidth;

            // lookup the black key to the left of the selected white key
            int bkeyLookup = BLACKKEY_LEFTOF[wkey];
            bool bkeyFound = false;

            // check for the black key on the left side of wkeyIndexNew
            if (bkeyLookup != KEYINDEX_NULL && wkeyx <= bkhalf) {
                bkeyFound = true;
            } else {
                // lookup the black key to the right of the selected white key
                bkeyLookup = BLACKKEY_LEFTOF[wkey == N_WHITEKEYS-1 ? 0 : wkey + 1];

                // check for the black key on the right side of wkeyIndexNew
                if (bkeyLookup != KEYINDEX_NULL && wkeyx >= wkwidth - bkhalf) {
                    bkeyFound = true;
                }
            }

            if (bkeyFound) {
                bkeyIndexNew = (octave * N_BLACKKEYS) + bkeyLookup;
                wkeyIndexNew = KEYINDEX_NULL;
            }
        }
    }

    bool newkey = false;
    if (wkeyIndex != wkeyIndexNew) {
        wkeyIndex = wkeyIndexNew;
        newkey = true;
    }
    if (bkeyIndex != bkeyIndexNew) {
        bkeyIndex = bkeyIndexNew;
        newkey = true;
    }

    return newkey;

}

trackerboy::Note PianoWidget::note() {
    
    int index;
    int divisor;
    const int *lookuptable;
    if (wkeyIndex != KEYINDEX_NULL) {
        index = wkeyIndex;
        divisor = N_WHITEKEYS;
        lookuptable = WHITEKEY_TO_NOTE;
    } else if (bkeyIndex != KEYINDEX_NULL) {
        index = bkeyIndex;
        divisor = N_BLACKKEYS;
        lookuptable = BLACKKEY_TO_NOTE;
    }

    int octave = index / divisor;
    int offset = lookuptable[index % divisor];
    return static_cast<trackerboy::Note>((octave * 12) + offset);
}



}
