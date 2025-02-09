
#include "graphics/CellPainter.hpp"

#include <QFontMetrics>

#define TU CellPainterTU
namespace TU {

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

// hexadecimal, 0-9, A-F
// notes A to G, b, #, 2-8
// effects: BCDFTEVIHSGL012345PQR
static const char PAINTABLE_CHARS[] = "ABCDEFGHTVIHSLPQR0123456789? -#b";
static constexpr int PAINTABLE_CHARS_COUNT = sizeof(PAINTABLE_CHARS) - 1;

}

CellPainter::CellPainter() :
    mCellHeight(0),
    mCellWidth(0),
    mCellScratch(1, '\0')
{
}

int CellPainter::cellHeight() const {
    return mCellHeight;
}

int CellPainter::cellWidth() const {
    return mCellWidth;
}

void CellPainter::setFont(QFont const& font) {
    QFontMetrics metrics(font);

    // get the bounding rect for the string of all paintable characters
    // tightBoundingRect is used to remove the spacing between lines
    auto rect = metrics.tightBoundingRect(TU::PAINTABLE_CHARS);
    // row height is the maximum height with 2 pixels padding for the cursor outline
    mCellHeight = rect.height() + 2;

    // get the average character width
    mCellWidth = metrics.size(Qt::TextSingleLine, TU::PAINTABLE_CHARS).width() / TU::PAINTABLE_CHARS_COUNT;
}


int CellPainter::drawCell(QPainter &painter, char cell, int xpos, int ypos) const {
    mCellScratch[0] = cell;
    painter.drawText(xpos, ypos, mCellWidth, mCellHeight, Qt::AlignBottom, mCellScratch);
    return xpos + mCellWidth;
}

int CellPainter::drawHex(QPainter &painter, int hex, int xpos, int ypos) const {
    xpos = drawCell(painter, TU::HEX_TABLE[(hex >> 4) & 0xF], xpos, ypos);
    return drawCell(painter, TU::HEX_TABLE[hex & 0xF], xpos, ypos);
}

int CellPainter::drawDec(QPainter &painter, int dec, int xpos, int ypos) const {
    xpos = drawCell(painter, '0' + (dec / 100), xpos, ypos);
    dec %= 100;
    xpos = drawCell(painter, '0' + (dec / 10), xpos, ypos);
    dec %= 10;
    return drawCell(painter, '0' + dec, xpos, ypos);
}

int CellPainter::calculateRowsAvailable(int height) const {
    return (height - 1) / mCellHeight + 1;
}

#undef TU
