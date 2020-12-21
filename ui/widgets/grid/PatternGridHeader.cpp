
#include "widgets/grid/PatternGridHeader.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

PatternGridHeader::PatternGridHeader(QWidget *parent) :
    QWidget(parent),
    mMetrics(),
    mHeaderFont(QStringLiteral(":/images/gridHeaderFont.bmp")),
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

void PatternGridHeader::setMetrics(PatternMetrics const& metrics) {
    mMetrics = metrics;
    update();
}


void PatternGridHeader::paintEvent(QPaintEvent *evt) {

    Q_UNUSED(evt);

    QPainter painter(this);
    auto &pal = palette();
    
    painter.setPen(pal.color(COLOR_FOREGROUND));
    painter.drawLine(0, HEIGHT - 2, width(), HEIGHT - 2);
    
    
    painter.translate(QPoint(mMetrics.offset, 0));

   

    // disabled tracks
    
    int xpos = mMetrics.rownoWidth;
    auto &disabledColor = pal.color(COLOR_DISABLED);
    for (int i = 0; i != 4; ++i) {
        if (!!(mTrackFlags & (1 << i))) {
            painter.fillRect(xpos, 0, mMetrics.trackWidth, HEIGHT, disabledColor);
        }
        xpos += mMetrics.trackWidth;
    }
    
    

    painter.translate(QPoint(mMetrics.rownoWidth, 0));

    xpos = 2;
    for (int i = 0; i != 4; ++i) {
        // draw "CH"
        painter.drawPixmap(xpos, 4, mHeaderFont, 0, 0, FONT_WIDTH * 2, FONT_HEIGHT);

        // draw number
        painter.drawPixmap(xpos + (FONT_WIDTH * 2), 4, mHeaderFont, FONT_WIDTH * (2 + i), 0, FONT_WIDTH, FONT_HEIGHT);
        xpos += mMetrics.trackWidth;
    }

    // lines
    xpos = 0;
    painter.setPen(pal.color(COLOR_LINE));
    for (int i = 0; i != 5; ++i) {
        painter.drawLine(xpos, 0, xpos, HEIGHT);
        xpos += mMetrics.trackWidth;
    }


    // highlight
    if (mTrackHover != HOVER_NONE) {
        painter.setPen(pal.color(COLOR_HOVER));
        int trackBegin = mMetrics.trackWidth * mTrackHover;
        int trackEnd = trackBegin + mMetrics.trackWidth;
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
    int translated = mx - mMetrics.offset - mMetrics.rownoWidth;
    int hover;

    if (translated > 0 && translated < mMetrics.trackWidth * 4) {
        hover = translated / mMetrics.trackWidth;
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

