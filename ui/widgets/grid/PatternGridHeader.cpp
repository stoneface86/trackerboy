
#include "widgets/grid/PatternGridHeader.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

PatternGridHeader::PatternGridHeader(QWidget *parent) :
    QWidget(parent),
    mHeaderFont(QStringLiteral(":/images/gridHeaderFont.bmp")),
    mOffset(0),
    mRownoWidth(0),
    mTrackWidth(0),
    mTrackHover(HOVER_NONE),
    mTrackFlags(0)
{
    setFixedHeight(HEIGHT);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void PatternGridHeader::setColors(ColorTable const& colorTable) {
    
    auto pal = palette();
    pal.setColor(COLOR_BACKGROUND, colorTable[+Color::headerBackground]);
    pal.setColor(COLOR_FOREGROUND, colorTable[+Color::headerForeground]);
    pal.setColor(COLOR_HOVER, colorTable[+Color::headerHover]);
    pal.setColor(COLOR_DISABLED, colorTable[+Color::headerDisabled]);
    pal.setColor(COLOR_LINE, colorTable[+Color::line]);
    setPalette(pal);

    update();
}

void PatternGridHeader::setWidths(int rownoWidth, int trackWidth) {
    mRownoWidth = rownoWidth;
    mTrackWidth = trackWidth;
    update();
}

void PatternGridHeader::setOffset(int offset) {
    mOffset = offset;
    update();
}


void PatternGridHeader::paintEvent(QPaintEvent *evt) {

    Q_UNUSED(evt);

    QPainter painter(this);
    auto &pal = palette();
    
    painter.setPen(pal.color(COLOR_FOREGROUND));
    painter.drawLine(0, HEIGHT - 2, width(), HEIGHT - 2);
    
    
    painter.translate(QPoint(mOffset, 0));

   

    // disabled tracks
    
    int xpos = mRownoWidth;
    auto &disabledColor = pal.color(COLOR_DISABLED);
    for (int i = 0; i != 4; ++i) {
        if (!!(mTrackFlags & (1 << i))) {
            painter.fillRect(xpos, 0, mTrackWidth, HEIGHT, disabledColor);
        }
        xpos += mTrackWidth;
    }
    
    

    painter.translate(QPoint(mRownoWidth, 0));

    xpos = 2;
    for (int i = 0; i != 4; ++i) {
        // draw "CH"
        painter.drawPixmap(xpos, 4, mHeaderFont, 0, 0, FONT_WIDTH * 2, FONT_HEIGHT);

        // draw number
        painter.drawPixmap(xpos + (FONT_WIDTH * 2), 4, mHeaderFont, FONT_WIDTH * (2 + i), 0, FONT_WIDTH, FONT_HEIGHT);
        xpos += mTrackWidth;
    }

    // lines
    xpos = 0;
    painter.setPen(pal.color(COLOR_LINE));
    for (int i = 0; i != 5; ++i) {
        painter.drawLine(xpos, 0, xpos, HEIGHT);
        xpos += mTrackWidth;
    }


    // highlight
    if (mTrackHover != HOVER_NONE) {
        painter.setPen(pal.color(COLOR_HOVER));
        int trackBegin = mTrackWidth * mTrackHover;
        int trackEnd = trackBegin + mTrackWidth;
        painter.drawLine(trackBegin, HEIGHT - 1, trackEnd, HEIGHT - 1);

    }
    

}

void PatternGridHeader::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);

    setTrackHover(HOVER_NONE);
}

void PatternGridHeader::mouseDoubleClickEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton && mTrackHover != HOVER_NONE) {

        if (mTrackFlags == 0xF) {
            // unsolo
            mTrackFlags = 0;
        } else {
            // solo
            mTrackFlags = (~(1 << mTrackHover)) & 0xF;
        }

        update();
    }
}

void PatternGridHeader::mouseMoveEvent(QMouseEvent *evt) {

    int mx = evt->x();

    // translate the x coordinate so that 0 = track 1
    int translated = mx - mOffset - mRownoWidth;
    int hover;

    if (translated > 0 && translated < mTrackWidth * 4) {
        hover = translated / mTrackWidth;
    } else {
        hover = HOVER_NONE;
    }
    setTrackHover(hover);
}

void PatternGridHeader::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton && mTrackHover != HOVER_NONE) {
        // user clicked on a track header either to mute or unmute the channel
        mTrackFlags ^= 1 << mTrackHover;
        update();
    }
}

void PatternGridHeader::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        update();
    }
}

