
#include "core/graphics/PatternPainter.hpp"
#include "core/model/PatternModel.hpp"

#include "trackerboy/note.hpp"

#define TU PatternPainterTU
namespace TU {

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

bool PatternPainter::flats() const {
    return mNoteTable == &NOTE_TABLE_FLATS;
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

void PatternPainter::drawRowBackground(QPainter &p, PatternLayout const& l, RowType type, int row) const {
    auto const _cellHeight = cellHeight();
    auto ypos = row * _cellHeight;

    auto const& color = mRowColors[type];
    p.setPen(mPen.get(color));
    
    auto const start = l.patternStart();
    auto const rowWidth = l.rowWidth();
    auto const lineEnd = start + rowWidth;
    
    p.drawLine(start, ypos, lineEnd, ypos);
    p.fillRect(start, ypos, rowWidth, _cellHeight, color);
    auto bottom = ypos + _cellHeight - 1;
    p.drawLine(start, bottom, lineEnd, bottom);
}

void PatternPainter::drawBackground(QPainter &p, PatternLayout const& l, int ypos, int rowStart, int rows) const {
    auto const _cellHeight = cellHeight();

    int rowno = rowStart;
    auto const start = l.patternStart();
    auto const _rowWidth = l.rowWidth();
    for (int i = 0; i < rows; ++i) {
        p.fillRect(start, ypos, _rowWidth, _cellHeight, mBackgroundColors[highlightIndex(rowno)]);
        ypos += _cellHeight;
        ++rowno;
    }
}

void PatternPainter::drawCursor(QPainter &p, PatternLayout const& l, PatternCursor cursor) const {
    auto const _cellWidth = cellWidth();
    auto const _cellHeight = cellHeight();

    // the width of the cursor is always 1 character unless it is over a note column, then it is 3
    int cursorWidth = (cursor.column == PatternCursor::ColumnNote ? 3 : 1) * _cellWidth;
    int cursorPos = l.trackToX(cursor.track) + l.columnToX(cursor.column);

    int ypos = cursor.row * _cellHeight;
    p.fillRect(cursorPos, ypos, cursorWidth, _cellHeight, mColorCursor);
    p.setPen(mPen.get(mColorCursor));
    p.drawRect(cursorPos, ypos, cursorWidth - 1, _cellHeight - 1);
}

void PatternPainter::drawLines(QPainter &p, PatternLayout const& l, int height) const {
    p.setPen(mPen.get(mColorLine));
    p.drawLine(0, 0, 0, height);
    int xpos = l.patternStart() - PatternLayout::LINE_WIDTH;
    p.drawLine(xpos, 0, xpos, height);

    for (int i = 0; i <= 3; ++i) {
        xpos += l.trackWidth(i) + PatternLayout::LINE_WIDTH;
        p.drawLine(xpos, 0, xpos, height);
    }

}

int PatternPainter::drawPattern(
    QPainter &p,
    PatternLayout const& l,
    trackerboy::Pattern const& pattern,
    int rowStart,
    int rowEnd,
    int ypos
) const {
    auto const _cellHeight = cellHeight();
    auto const start = l.patternStart();

    // text centering
    ypos++;

    for (int rowno = rowStart; rowno <= rowEnd; ++rowno) {
        auto const& fgcolor = mForegroundColors[highlightIndex(rowno)];
        p.setPen(mPen.get(fgcolor));
        drawHex(p, rowno, PatternLayout::SPACING, ypos);
        int xpos = start + PatternLayout::SPACING;
        for (int track = 0; track <= 3; ++track) {
            auto &trackdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), rowno);

            auto note = trackdata.queryNote();
            if (note) {
                xpos = drawNote(p, *note, xpos, ypos);
            } else {
                xpos = drawNone(p, 3, xpos, ypos);
            }

            xpos += PatternLayout::SPACING;
            auto instrument = trackdata.queryInstrument();
            if (instrument) {
                p.setPen(mPen.get(mColorInstrument));
                xpos = drawHex(p, *instrument, xpos, ypos);
                p.setPen(mPen.get(fgcolor));
            } else {
                xpos = drawNone(p, 2, xpos, ypos);
            }

            xpos += PatternLayout::SPACING;

            auto const effectsVisible = l.effectsVisible(track);
            for (int effect = 0; effect < effectsVisible; ++effect) {
                auto effectdata = trackdata.effects[effect];
                if (effectdata.type != trackerboy::EffectType::noEffect) {
                    p.setPen(mPen.get(mColorEffect));

                    xpos = drawCell(p, TU::effectTypeToChar(effectdata.type), xpos, ypos);

                    p.setPen(mPen.get(fgcolor));
                    xpos = drawHex(p, effectdata.param, xpos, ypos);
                } else {
                    xpos = drawNone(p, 3, xpos, ypos);
                }

                xpos += PatternLayout::SPACING;

            }

            xpos += PatternLayout::LINE_WIDTH + PatternLayout::SPACING;
        }

        ypos += _cellHeight;
    }

    return ypos - 1;
}

void PatternPainter::drawSelection(QPainter &painter, QRect const& rect) const {
    painter.fillRect(rect, mColorSelection);
}

int PatternPainter::drawNote(QPainter &painter, uint8_t note, int xpos, int ypos) const {
    auto const _cellWidth = cellWidth();

    if (note == trackerboy::NOTE_CUT) {
        painter.fillRect(xpos, ypos + cellHeight() / 2, _cellWidth * 2, 4, painter.pen().color());
        return xpos + _cellWidth * 3;
    } else {

        int octave = note / 12;
        int key = note % 12;
        octave += 2;

        auto notestr = mNoteTable->data() + (key * 2);
        xpos = drawCell(painter, *notestr++, xpos, ypos);
        xpos = drawCell(painter, *notestr, xpos, ypos);
        return drawCell(painter, octave + '0', xpos, ypos);
    }
}


int PatternPainter::drawNone(QPainter &painter, int cells, int xpos, int ypos) const {
    auto const _cellWidth = cellWidth();

    xpos += 3;
    ypos += cellHeight() / 2;
    int const width = _cellWidth - 6;
    for (int i = cells; i--; ) {
        painter.drawLine(xpos, ypos, xpos + width, ypos);
        xpos += _cellWidth;
    }
    return xpos - 3;
}

int PatternPainter::highlightIndex(int rowno) const {
    // this could probably be optimized better
    if (mHighlightInterval2 && rowno % mHighlightInterval2 == 0) {
        return 2;
    } else if (mHighlightInterval1 && rowno % mHighlightInterval1 == 0) {
        return 1;
    } else {
        return 0;
    }
}

#undef TU
