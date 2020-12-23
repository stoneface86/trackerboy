
#include "widgets/grid/PatternPainter.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/note.hpp"

#include <QFontMetrics>

using namespace PatternConstants;

static const char HEX_TABLE[16] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
};

// TODO: move this somewhere in the library
static char effectTypeToChar(trackerboy::EffectType et) {

    switch (et) {
        case trackerboy::EffectType::patternGoto:
            return 'B';
        case trackerboy::EffectType::patternHalt:
            return 'C';
        case trackerboy::EffectType::patternSkip:
            return 'D';
        case trackerboy::EffectType::setTempo:
            return 'F';
        case trackerboy::EffectType::sfx:
            return 'T';
        case trackerboy::EffectType::setEnvelope:
            return 'E';
        case trackerboy::EffectType::setTimbre:
            return 'V';
        case trackerboy::EffectType::setPanning:
            return 'I';
        case trackerboy::EffectType::setSweep:
            return 'H';
        case trackerboy::EffectType::delayedCut:
            return 'S';
        case trackerboy::EffectType::delayedNote:
            return 'G';
        case trackerboy::EffectType::lock:
            return 'L';
        case trackerboy::EffectType::arpeggio:
            return '0';
        case trackerboy::EffectType::pitchUp:
            return '1';
        case trackerboy::EffectType::pitchDown:
            return '2';
        case trackerboy::EffectType::autoPortamento:
            return '3';
        case trackerboy::EffectType::vibrato:
            return '4';
        case trackerboy::EffectType::vibratoDelay:
            return '5';
        case trackerboy::EffectType::tuning:
            return 'P';
        case trackerboy::EffectType::noteSlideUp:
            return 'Q';
        case trackerboy::EffectType::noteSlideDown:
            return 'R';
        default:
            return '?';
    }
}



PatternPainter::PatternPainter(QFont const& font) :
    mHighlightInterval1(0),
    mHighlightInterval2(0),
    mCellWidth(0),
    mCellHeight(0),
    mRownoWidth(0),
    mTrackWidth(0),
    mPatternWidth(0),
    mDisplayFlats(false)
{
    setFont(font);
}

int PatternPainter::cellHeight() const {
    return mCellHeight;
}

int PatternPainter::cellWidth() const {
    return mCellWidth;
}

int PatternPainter::rownoWidth() const {
    return mRownoWidth;
}

int PatternPainter::trackWidth() const {
    return mTrackWidth;
}

bool PatternPainter::flats() const {
    return mDisplayFlats;
}

void PatternPainter::setFont(QFont const& font) {
    QFontMetrics metrics(font);

    // hexadecimal, 0-9, A-F
    // notes A to G, b, #, 2-8
    // effects: BCDFTEVIHSGL012345PQR
    static const char PAINTABLE_CHARS[] = "ABCDEFGHTVIHSLPQR0123456789? -#b";
    constexpr int PAINTABLE_CHARS_COUNT = sizeof(PAINTABLE_CHARS) - 1;

    // get the bounding rect for the string of all paintable characters
    // tightBoundingRect is used to remove the spacing between lines
    auto rect = metrics.tightBoundingRect(PAINTABLE_CHARS);
    // row height is the maximum height with 2 pixels padding for the cursor outline
    mCellHeight = rect.height() + 2;

    // get the average character width
    mCellWidth = metrics.size(Qt::TextSingleLine, PAINTABLE_CHARS).width() / PAINTABLE_CHARS_COUNT;

    mRownoWidth = mCellWidth * ROWNO_CELLS;
    mTrackWidth = mCellWidth * TRACK_CELLS;
    mPatternWidth = mTrackWidth * 4;
}

void PatternPainter::setFirstHighlight(int interval) {
    assert(interval >= 0);
    mHighlightInterval1 = interval;
}

void PatternPainter::setSecondHighlight(int interval) {
    assert(interval >= 0);
    mHighlightInterval2 = interval;
}

void PatternPainter::setFlats(bool flats) {
    mDisplayFlats = flats;
}

void PatternPainter::setColors(ColorTable const& colors) {
    mColorForeground = colors[+Color::foreground];
    mColorForegroundHighlight1 = colors[+Color::foregroundHighlight1];
    mColorForegroundHighlight2 = colors[+Color::foregroundHighlight2];
    mColorBackground = colors[+Color::background];
    mColorBackground1 = colors[+Color::backgroundHighlight1];
    mColorBackground2 = colors[+Color::backgroundHighlight2];
    mColorInstrument = colors[+Color::instrument];
    mColorEffect = colors[+Color::effectType];
    mColorSelection = colors[+Color::selection];
    mColorCursor = colors[+Color::cursor];
    mColorCursor.setAlpha(128);
    mColorLine = colors[+Color::line];
}


int PatternPainter::columnLocation(int column) const {
    int track = column / TRACK_COLUMNS;
    int coltype = column % TRACK_COLUMNS;
    return track * mTrackWidth + TRACK_COLUMN_MAP[coltype] * mCellWidth + mRownoWidth;
}

void PatternPainter::drawCursor(QPainter &painter, int row, int column) {
    // the width of the cursor is always 1 character unless it is over a note column, then it is 3
    int cursorWidth = ((column % TRACK_COLUMNS) == COLUMN_NOTE ? 3 : 1) * mCellWidth + 2;
    int cursorPos = columnLocation(column) - 1;

    int ypos = row * mCellHeight;
    painter.fillRect(cursorPos, ypos, cursorWidth, mCellHeight, mColorCursor);
    painter.setPen(mColorCursor);
    painter.drawRect(cursorPos, ypos, cursorWidth - 1, mCellHeight - 1);
}

void PatternPainter::drawLines(QPainter &painter, int height) {
    painter.setPen(mColorLine);
    painter.drawLine(0, 0, 0, height);
    int xpos = mRownoWidth;
    for (int i = 0; i != 5; ++i) {
        painter.drawLine(xpos, 0, xpos, height);
        xpos += mTrackWidth;
    }
}


int PatternPainter::drawRow(
    QPainter &painter,
    trackerboy::PatternRow const& rowdata,
    int rowno,
    int ypos
) {

    QPen fgpen(foregroundColor(rowno));

    // text centering
    ypos++;

    painter.setPen(fgpen);
    painter.drawText(mCellWidth, ypos, mCellWidth * 2, mCellHeight, Qt::AlignBottom, QString("%1").arg(rowno, 2, 16, QLatin1Char('0')).toUpper());
    int xpos = (TRACK_COLUMN_MAP[COLUMN_NOTE] + ROWNO_CELLS) * mCellWidth;
    for (int track = 0; track != 4; ++track) {
        auto &trackdata = rowdata[track];

        if (!!(trackdata.flags & trackerboy::TrackRow::COLUMN_NOTE)) {
            drawNote(painter, trackdata.note, xpos, ypos);
        } else {
            drawNone(painter, 3, xpos, ypos);
        }


        xpos += (TRACK_COLUMN_MAP[COLUMN_INSTRUMENT_HIGH] - TRACK_COLUMN_MAP[COLUMN_NOTE]) * mCellWidth;
        if (!!(trackdata.flags & trackerboy::TrackRow::COLUMN_INST)) {
            uint8_t inst = trackdata.instrumentId;
            painter.setPen(mColorInstrument);
            drawCell(painter, HEX_TABLE[inst >> 4], xpos, ypos);
            drawCell(painter, HEX_TABLE[inst & 0xF], xpos + mCellWidth, ypos);
            painter.setPen(fgpen);
        } else {
            drawNone(painter, 2, xpos, ypos);
        }

        xpos += (TRACK_COLUMN_MAP[COLUMN_EFFECT1_TYPE] - TRACK_COLUMN_MAP[COLUMN_INSTRUMENT_HIGH]) * mCellWidth;
        int effectFlag = trackerboy::TrackRow::COLUMN_EFFECT1;
        for (int effect = 0; effect < trackerboy::TrackRow::MAX_EFFECTS; ++effect) {
            if (!!(trackdata.flags & effectFlag)) {
                auto effectdata = trackdata.effects[effect];
                painter.setPen(mColorEffect);

                drawCell(painter, effectTypeToChar(effectdata.type), xpos, ypos);
                xpos += mCellWidth;

                painter.setPen(fgpen);
                drawCell(painter, HEX_TABLE[effectdata.param >> 4], xpos, ypos);
                xpos += mCellWidth;
                drawCell(painter, HEX_TABLE[effectdata.param & 0xF], xpos, ypos);
                xpos += mCellWidth;
            } else {
                drawNone(painter, 3, xpos, ypos);
                xpos += mCellWidth * 3;
            }

            effectFlag <<= 1;

        }

        xpos += 2 * mCellWidth;
    }

    return ypos - 1 + mCellHeight;
}

void PatternPainter::drawColumn(QPainter &painter, trackerboy::PatternRow const& data, int cell, int ypos) {
    //int track = cell / TRACK_CELLS;
    //
    //auto col = static_cast<ColumnType>(TRACK_CELL_MAP[cell % TRACK_CELLS]);
    //int xpos = mRownoWidth + mTrackWidth;

    //
    //auto &trackrow = data[track];

    //bool note;
    //switch (col) {
    //    case COLUMN_NOTE:
    //        note = true;
    //        break;
    //}
}

void PatternPainter::drawNote(QPainter &painter, uint8_t note, int xpos, int ypos) {

    const char* NOTE_STR_SHARP[] = {
        "C-",
        "C#",
        "D-",
        "D#",
        "E-",
        "F-",
        "F#",
        "G-",
        "G#",
        "A-",
        "A#",
        "B-"
    };

    const char* NOTE_STR_FLAT[] = {
        "C-",
        "Db",
        "D-",
        "Eb",
        "E-",
        "F-",
        "Gb",
        "G-",
        "Ab",
        "A-",
        "Bb",
        "B-"
    };

    if (note == trackerboy::NOTE_CUT) {
        painter.setBrush(QBrush(painter.pen().color()));
        painter.drawRect(xpos, ypos + mCellHeight / 2, mCellWidth * 2, 2);
    } else {

        int octave = note / 12;
        int key = note % 12;
        octave += 2;

        auto notetable = (mDisplayFlats) ? NOTE_STR_FLAT : NOTE_STR_SHARP;

        painter.drawText(xpos, ypos, 3 * mCellWidth, mCellHeight, Qt::AlignBottom,
            QStringLiteral("%1%2").arg(notetable[key]).arg(octave));
    }
}


void PatternPainter::drawNone(QPainter &painter, int cells, int xpos, int ypos) {
    xpos += 3;
    ypos += mCellHeight / 2;
    int const width = mCellWidth - 6;
    for (int i = cells; i--; ) {
        painter.drawLine(xpos, ypos, xpos + width, ypos);
        xpos += mCellWidth;
    }
}

void PatternPainter::drawCell(QPainter &painter, char cell, int xpos, int ypos) {
    painter.drawText(xpos, ypos, mCellWidth, mCellHeight, Qt::AlignBottom, QString(cell));
}


QColor const& PatternPainter::foregroundColor(int rowno) {
    if (mHighlightInterval2 && rowno % mHighlightInterval2 == 0) {
        return mColorForegroundHighlight2;
    } else if (mHighlightInterval1 && rowno % mHighlightInterval1 == 0) {
        return mColorForegroundHighlight1;
    } else {
        return mColorForeground;
    }
}


