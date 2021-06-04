
#include "widgets/grid/PatternPainter.hpp"
#include "widgets/grid/layout.hpp"
#include "core/model/PatternModel.hpp"

#include "trackerboy/note.hpp"

#include <QFontMetrics>
#include <QtDebug>

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

static const char* NOTE_STR_SHARP[] = {
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

static const char* NOTE_STR_FLAT[] = {
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
    mForegroundColors[0] = colors[+Color::foreground];
    mForegroundColors[1] = colors[+Color::foregroundHighlight1];
    mForegroundColors[2] = colors[+Color::foregroundHighlight2];
    mBackgroundColors[0] = colors[+Color::background];
    mBackgroundColors[1] = colors[+Color::backgroundHighlight1];
    mBackgroundColors[2] = colors[+Color::backgroundHighlight2];
    mColorInstrument = colors[+Color::instrument];
    mColorEffect = colors[+Color::effectType];
    mColorSelection = colors[+Color::selection];
    mColorSelection.setAlpha(200);
    mColorCursor = colors[+Color::cursor];
    mColorCursor.setAlpha(128);
    mColorLine = colors[+Color::line];

    mRowColors[0] = colors[+Color::row];
    mRowColors[1] = colors[+Color::rowEdit];
    mRowColors[2] = colors[+Color::rowPlayer];
    for (auto &color : mRowColors) {
        color.setAlpha(128);
    }

}

void PatternPainter::drawRowBackground(QPainter &painter, RowType type, int row) {
    auto ypos = row * mCellHeight;
    auto const& color = mRowColors[type];
    painter.setPen(color);
    auto const lineEnd = mRownoWidth + mPatternWidth;
    painter.drawLine(mRownoWidth, ypos, lineEnd, ypos);
    painter.fillRect(mRownoWidth, ypos, mPatternWidth, mCellHeight, color);
    auto bottom = ypos + mCellHeight - 1;
    painter.drawLine(mRownoWidth, bottom, lineEnd, bottom);
}

void PatternPainter::drawBackground(QPainter &painter, int ypos, int rowStart, int rows) {
    int rowno = rowStart;
    for (int i = 0; i < rows; ++i) {
        painter.fillRect(mRownoWidth, ypos, mPatternWidth, mCellHeight, mBackgroundColors[highlightIndex(rowno)]);
        ypos += mCellHeight;
        ++rowno;
    }
}

void PatternPainter::drawCursor(QPainter &painter, PatternCursor cursor) {
    // the width of the cursor is always 1 character unless it is over a note column, then it is 3
    int cursorWidth = (cursor.column == PatternCursor::ColumnNote ? 3 : 1) * mCellWidth + 2;
    int cursorPos = (cursor.track * mTrackWidth) + (TRACK_COLUMN_MAP[cursor.column] * mCellWidth) + mRownoWidth - 1;

    int ypos = cursor.row * mCellHeight;
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

    QPen fgpen(mForegroundColors[highlightIndex(rowno)]);

    // text centering
    ypos++;

    painter.setPen(fgpen);
    painter.drawText(mCellWidth, ypos, mCellWidth * 2, mCellHeight, Qt::AlignBottom, QString("%1").arg(rowno, 2, 16, QLatin1Char('0')).toUpper());
    int xpos = (TRACK_COLUMN_MAP[PatternCursor::ColumnNote] + ROWNO_CELLS) * mCellWidth;
    for (int track = 0; track != 4; ++track) {
        auto &trackdata = rowdata[track];

        auto note = trackdata.queryNote();
        if (note) {
            drawNote(painter, *note, xpos, ypos);
        } else {
            drawNone(painter, 3, xpos, ypos);
        }


        xpos += (TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh] - TRACK_COLUMN_MAP[PatternCursor::ColumnNote]) * mCellWidth;
        auto instrument = trackdata.queryInstrument();
        if (instrument) {
            painter.setPen(mColorInstrument);
            drawCell(painter, HEX_TABLE[*instrument >> 4], xpos, ypos);
            drawCell(painter, HEX_TABLE[*instrument & 0xF], xpos + mCellWidth, ypos);
            painter.setPen(fgpen);
        } else {
            drawNone(painter, 2, xpos, ypos);
        }

        xpos += (TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type] - TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh]) * mCellWidth;

        for (int effect = 0; effect < trackerboy::TrackRow::MAX_EFFECTS; ++effect) {
            auto effectdata = trackdata.effects[effect];
            if (effectdata.type != trackerboy::EffectType::noEffect) {
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

            xpos += mCellWidth;

        }

        xpos += mCellWidth;
    }

    return ypos - 1 + mCellHeight;
}

QRect PatternPainter::selectionRectangle(PatternSelection const& selection) {
    auto iter = selection.iterator();
    int x1;
    {
        int cell;
        switch (iter.columnStart()) {
            case PatternSelection::SelectNote:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnNote] - 1;
                break;
            case PatternSelection::SelectInstrument:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh];
                break;
            case PatternSelection::SelectEffect1:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type];
                break;
            case PatternSelection::SelectEffect2:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect2Type];
                break;
            default:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect3Type];
                break;
        }
        x1 = mRownoWidth + (mCellWidth * ((iter.trackStart() * TRACK_CELLS) + cell));
    }
    int x2;
    {
        int cell;
        switch (iter.columnEnd()) {
            case PatternSelection::SelectNote:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnNote] + 3;
                break;
            case PatternSelection::SelectInstrument:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh] + 2;
                break;
            case PatternSelection::SelectEffect1:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type] + 3;
                break;
            case PatternSelection::SelectEffect2:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect2Type] + 3;
                break;
            default:
                cell = TRACK_COLUMN_MAP[PatternCursor::ColumnEffect3Type] + 4;
                break;
        }
        x2 = mRownoWidth + (mCellWidth * ((iter.trackEnd() * TRACK_CELLS) + cell));
    }

    return {x1, iter.rowStart() * mCellHeight, x2 - x1, iter.rows() * mCellHeight};
}


void PatternPainter::drawSelection(QPainter &painter, PatternSelection const& selection) {
    painter.fillRect(selectionRectangle(selection), mColorSelection);
    
    //painter.setPen(mColorSelection);
    //painter.drawRect(x1, ypos, width - 1, height - 1);

}

void PatternPainter::drawNote(QPainter &painter, uint8_t note, int xpos, int ypos) {

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


int PatternPainter::highlightIndex(int rowno) {
    if (mHighlightInterval2 && rowno % mHighlightInterval2 == 0) {
        return 2;
    } else if (mHighlightInterval1 && rowno % mHighlightInterval1 == 0) {
        return 1;
    } else {
        return 0;
    }
}


