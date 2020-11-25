
#include "widgets/grid/PatternGridHeader.hpp"

#include <QPainter>
#include <QtDebug>

constexpr int HEIGHT = 32;

constexpr int FONT_WIDTH = 7;
constexpr int FONT_HEIGHT = 11;

constexpr int FONT_CHAR1 = FONT_WIDTH * 2;
constexpr int FONT_CHAR2 = FONT_WIDTH * 3;
constexpr int FONT_CHAR3 = FONT_WIDTH * 4;
constexpr int FONT_CHAR4 = FONT_WIDTH * 5;

namespace grid {

PatternGridHeader::PatternGridHeader(QWidget *parent) :
    QWidget(parent),
    mFontBitmap(":/icons/gridHeaderFont.bmp"),
    mColorBg(224, 248, 208),
    mColorFg(8, 24, 32),
    mColorDisabled(52, 104, 86),
    mColorHighlight(136, 192, 112)
{
    setFixedHeight(HEIGHT);
}



void PatternGridHeader::paintEvent(QPaintEvent *evt) {

    QPainter painter(this);

    // background
    painter.fillRect(rect(), mColorBg);

    // disabled tracks
    // TODO

    // highlight

    if (mMouseOverTrack != -1) {

    }


    painter.setPen(mColorFg);
    painter.drawLine(0, HEIGHT - 2, width(), HEIGHT - 2);



    // lines + text
    //painter.translate(mMetrics.xoffset, 0);
    painter.drawPixmap(0, 0, mDisplay);

    // draw buttons


    // draw volume meters
}

}
