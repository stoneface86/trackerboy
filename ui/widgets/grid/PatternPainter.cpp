
#include "widgets/grid/PatternPainter.hpp"
#include "widgets/grid/layout.hpp"
#include "core/model/PatternModel.hpp"

#include "trackerboy/note.hpp"

#define TU PatternPainterTU
namespace TU {

using namespace PatternConstants;

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

} // namespace TU

PatternPainter::NoteTable const PatternPainter::NOTE_TABLE_FLATS = {
    'C', '-',   // note  0: C-
    'D', 'b',   // note  1: Db
    'D', '-',   // note  2: D-
    'E', 'b',   // note  3: Eb
    'E', '-',   // note  4: E-
    'F', '-',   // note  5: F-
    'G', 'b',   // note  6: Gb
    'G', '-',   // note  7: G-
    'A', 'b',   // note  8: Ab
    'A', '-',   // note  9: A-
    'B', 'b',   // note 10: Bb
    'B', '-'    // note 11: B-
};

PatternPainter::NoteTable const PatternPainter::NOTE_TABLE_SHARPS = {
    'C', '-',   // note  0: C-
    'C', '#',   // note  1: C#
    'D', '-',   // note  2: D-
    'D', '#',   // note  3: D#
    'E', '-',   // note  4: E-
    'F', '-',   // note  5: F-
    'F', '#',   // note  6: F#
    'G', '-',   // note  7: G-
    'G', '#',   // note  8: G#
    'A', '-',   // note  9: A-
    'A', '#',   // note 10: A#
    'B', '-'    // note 11: B-
};

// NOTE
// Do not create temporary QPens! The member variable, mPen, should be used
// instead when needing to modify QPainter's pen. Doing so will prevent
// unnecessary heap allocations and improve performance.


PatternPainter::PatternPainter(QFont const& font) :
    CellPainter(),
    mHighlightInterval1(0),
    mHighlightInterval2(0),
    mRownoWidth(0),
    mTrackWidth(0),
    mPatternWidth(0),
    mNoteTable(&NOTE_TABLE_SHARPS),
    mForegroundColors(),
    mBackgroundColors(),
    mColorInstrument(),
    mColorEffect(),
    mColorSelection(),
    mColorCursor(),
    mColorLine(),
    mRowColors(),
    mPen()
{
    setFont(font);
}

int PatternPainter::rownoWidth() const {
    return mRownoWidth;
}

int PatternPainter::trackWidth() const {
    return mTrackWidth;
}

bool PatternPainter::flats() const {
    return mNoteTable == &NOTE_TABLE_FLATS;
}

void PatternPainter::cellSizeChanged(int width, int height) {
    Q_UNUSED(height)

    mRownoWidth = width * TU::ROWNO_CELLS;
    mTrackWidth = width * TU::TRACK_CELLS;
    mPatternWidth = mTrackWidth * 4;
}

void PatternPainter::setFirstHighlight(int interval) {
    Q_ASSERT(interval >= 0);
    mHighlightInterval1 = interval;
}

void PatternPainter::setSecondHighlight(int interval) {
    Q_ASSERT(interval >= 0);
    mHighlightInterval2 = interval;
}

void PatternPainter::setFlats(bool flats) {
    mNoteTable = (flats) ? &NOTE_TABLE_FLATS : &NOTE_TABLE_SHARPS;
}

void PatternPainter::setColors(Palette const& colors) {
    mForegroundColors[0] = colors[Palette::ColorForeground];
    mForegroundColors[1] = colors[Palette::ColorForegroundHighlight1];
    mForegroundColors[2] = colors[Palette::ColorForegroundHighlight2];
    mBackgroundColors[0] = colors[Palette::ColorBackground];
    mBackgroundColors[1] = colors[Palette::ColorBackgroundHighlight1];
    mBackgroundColors[2] = colors[Palette::ColorBackgroundHighlight2];
    mColorInstrument     = colors[Palette::ColorInstrument];
    mColorEffect         = colors[Palette::ColorEffectType];
    mColorSelection      = colors[Palette::ColorSelection];
    mColorSelection.setAlpha(200);
    mColorCursor         = colors[Palette::ColorCursor];
    mColorCursor.setAlpha(128);
    mColorLine           = colors[Palette::ColorLine];

    mRowColors[0] = colors[Palette::ColorRow];
    mRowColors[1] = colors[Palette::ColorRowEdit];
    mRowColors[2] = colors[Palette::ColorRowPlayer];
    for (auto &color : mRowColors) {
        color.setAlpha(128);
    }

}

void PatternPainter::drawRowBackground(QPainter &painter, RowType type, int row) {
    auto const _cellHeight = cellHeight();

    auto ypos = row * _cellHeight;
    auto const& color = mRowColors[type];
    painter.setPen(pen(color));
    auto const lineEnd = mRownoWidth + mPatternWidth;
    painter.drawLine(mRownoWidth, ypos, lineEnd, ypos);
    painter.fillRect(mRownoWidth, ypos, mPatternWidth, _cellHeight, color);
    auto bottom = ypos + _cellHeight - 1;
    painter.drawLine(mRownoWidth, bottom, lineEnd, bottom);
}

void PatternPainter::drawBackground(QPainter &painter, int ypos, int rowStart, int rows) {
    auto const _cellHeight = cellHeight();

    int rowno = rowStart;
    for (int i = 0; i < rows; ++i) {
        painter.fillRect(mRownoWidth, ypos, mPatternWidth, _cellHeight, mBackgroundColors[highlightIndex(rowno)]);
        ypos += _cellHeight;
        ++rowno;
    }
}

void PatternPainter::drawCursor(QPainter &painter, PatternCursor cursor) {
    auto const _cellWidth = cellWidth();
    auto const _cellHeight = cellHeight();

    // the width of the cursor is always 1 character unless it is over a note column, then it is 3
    int cursorWidth = (cursor.column == PatternCursor::ColumnNote ? 3 : 1) * _cellWidth + 2;
    int cursorPos = (cursor.track * mTrackWidth) + (TU::TRACK_COLUMN_MAP[cursor.column] * _cellWidth) + mRownoWidth - 1;

    int ypos = cursor.row * _cellHeight;
    painter.fillRect(cursorPos, ypos, cursorWidth, _cellHeight, mColorCursor);
    painter.setPen(pen(mColorCursor));
    painter.drawRect(cursorPos, ypos, cursorWidth - 1, _cellHeight - 1);
}

void PatternPainter::drawLines(QPainter &painter, int height) {
    painter.setPen(pen(mColorLine));
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

    auto const _cellWidth = cellWidth();
    auto const _cellHeight = cellHeight();

    auto const& fgcolor = mForegroundColors[highlightIndex(rowno)];

    // text centering
    ypos++;

    painter.setPen(pen(fgcolor));
    drawHex(painter, rowno, _cellWidth, ypos);
    int xpos = (TU::TRACK_COLUMN_MAP[PatternCursor::ColumnNote] + TU::ROWNO_CELLS) * _cellWidth;
    for (int track = 0; track != 4; ++track) {
        auto &trackdata = rowdata[track];

        auto note = trackdata.queryNote();
        if (note) {
            drawNote(painter, *note, xpos, ypos);
        } else {
            drawNone(painter, 3, xpos, ypos);
        }


        xpos += (TU::TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh] - TU::TRACK_COLUMN_MAP[PatternCursor::ColumnNote]) * _cellWidth;
        auto instrument = trackdata.queryInstrument();
        if (instrument) {
            painter.setPen(pen(mColorInstrument));
            drawHex(painter, *instrument, xpos, ypos);
            painter.setPen(pen(fgcolor));
        } else {
            drawNone(painter, 2, xpos, ypos);
        }

        xpos += (TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type] - TU::TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh]) * _cellWidth;

        for (int effect = 0; effect < trackerboy::TrackRow::MAX_EFFECTS; ++effect) {
            auto effectdata = trackdata.effects[effect];
            if (effectdata.type != trackerboy::EffectType::noEffect) {
                painter.setPen(pen(mColorEffect));

                drawCell(painter, TU::effectTypeToChar(effectdata.type), xpos, ypos);
                xpos += _cellWidth;

                painter.setPen(pen(fgcolor));
                drawHex(painter, effectdata.param, xpos, ypos);
                xpos += _cellWidth * 2;
            } else {
                drawNone(painter, 3, xpos, ypos);
                xpos += _cellWidth * 3;
            }

            xpos += _cellWidth;

        }

        xpos += _cellWidth;
    }

    return ypos - 1 + _cellHeight;
}

QRect PatternPainter::selectionRectangle(PatternSelection const& selection) {
    auto const _cellWidth = cellWidth();
    auto const _cellHeight = cellHeight();

    auto iter = selection.iterator();
    int x1;
    {
        int cell;
        switch (iter.columnStart()) {
            case PatternAnchor::SelectNote:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnNote] - 1;
                break;
            case PatternAnchor::SelectInstrument:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh];
                break;
            case PatternAnchor::SelectEffect1:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type];
                break;
            case PatternAnchor::SelectEffect2:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect2Type];
                break;
            default:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect3Type];
                break;
        }
        x1 = mRownoWidth + (_cellWidth * ((iter.trackStart() * TU::TRACK_CELLS) + cell));
    }
    int x2;
    {
        int cell;
        switch (iter.columnEnd()) {
            case PatternAnchor::SelectNote:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnNote] + 3;
                break;
            case PatternAnchor::SelectInstrument:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnInstrumentHigh] + 2;
                break;
            case PatternAnchor::SelectEffect1:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect1Type] + 3;
                break;
            case PatternAnchor::SelectEffect2:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect2Type] + 3;
                break;
            default:
                cell = TU::TRACK_COLUMN_MAP[PatternCursor::ColumnEffect3Type] + 4;
                break;
        }
        x2 = mRownoWidth + (_cellWidth * ((iter.trackEnd() * TU::TRACK_CELLS) + cell));
    }

    return {x1, iter.rowStart() * _cellHeight, x2 - x1, iter.rows() * _cellHeight};
}


void PatternPainter::drawSelection(QPainter &painter, PatternSelection const& selection) {
    painter.fillRect(selectionRectangle(selection), mColorSelection);
}

void PatternPainter::drawNote(QPainter &painter, uint8_t note, int xpos, int ypos) {
    auto const _cellWidth = cellWidth();

    if (note == trackerboy::NOTE_CUT) {
        painter.fillRect(xpos, ypos + cellHeight() / 2, _cellWidth * 2, 4, painter.pen().color());
    } else {

        int octave = note / 12;
        int key = note % 12;
        octave += 2;

        auto notestr = mNoteTable->data() + (key * 2);
        drawCell(painter, *notestr++, xpos, ypos);
        xpos += _cellWidth;
        drawCell(painter, *notestr, xpos, ypos);
        xpos += _cellWidth;
        drawCell(painter, octave + '0', xpos, ypos);
    }
}


void PatternPainter::drawNone(QPainter &painter, int cells, int xpos, int ypos) {
    auto const _cellWidth = cellWidth();

    xpos += 3;
    ypos += cellHeight() / 2;
    int const width = _cellWidth - 6;
    for (int i = cells; i--; ) {
        painter.drawLine(xpos, ypos, xpos + width, ypos);
        xpos += _cellWidth;
    }
}

int PatternPainter::highlightIndex(int rowno) {
    // this could probably be optimized better
    if (mHighlightInterval2 && rowno % mHighlightInterval2 == 0) {
        return 2;
    } else if (mHighlightInterval1 && rowno % mHighlightInterval1 == 0) {
        return 1;
    } else {
        return 0;
    }
}

QPen const& PatternPainter::pen(QColor const& color) {
    mPen.setColor(color);
    return mPen;
}

#undef TU
