
#include "widgets/grid/PatternGridHeader.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

PatternGridHeader::PatternGridHeader(QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mOffset(0),
    mRownoWidth(0),
    mTrackWidth(0),
    mHeaderFont(QStringLiteral(":/images/gridHeaderFont.bmp")),
    mTrackHover(HOVER_NONE),
    mTrackFlags(ModuleDocument::AllOn),
    mColorForeground(),
    mColorHover(),
    mColorDisabled(),
    mColorLine()
{
    setFixedHeight(HEIGHT);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void PatternGridHeader::setColors(ColorTable const& colorTable) {
    
    auto pal = palette();
    pal.setColor(backgroundRole(), colorTable[+Color::headerBackground]);
    setPalette(pal);
    mColorForeground = colorTable[+Color::headerForeground];
    mColorHover = colorTable[+Color::headerHover];
    mColorDisabled = colorTable[+Color::headerDisabled];
    mColorLine = colorTable[+Color::line];

    update();
}

void PatternGridHeader::setDocument(ModuleDocument *doc) {
    if (mDocument) {
        mDocument->disconnect(this);
    }

    mDocument = doc;
    if (doc) {
        mTrackFlags = doc->channelOutput();
        update();

        connect(doc, &ModuleDocument::channelOutputChanged, this, &PatternGridHeader::setOutputFlags);
    }
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
    
    painter.setPen(mColorForeground);
    painter.drawLine(0, HEIGHT - 2, width(), HEIGHT - 2);
    
    
    painter.translate(QPoint(mOffset, 0));

   

    // disabled tracks
    
    int xpos = mRownoWidth;
    for (int i = 0; i != 4; ++i) {
        if (!(mTrackFlags & (1 << i))) {
            painter.fillRect(xpos, 0, mTrackWidth, HEIGHT, mColorDisabled);
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
    painter.setPen(mColorLine);
    for (int i = 0; i != 5; ++i) {
        painter.drawLine(xpos, 0, xpos, HEIGHT);
        xpos += mTrackWidth;
    }


    // highlight
    if (mTrackHover != HOVER_NONE) {
        painter.setPen(mColorHover);
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

        if (mTrackFlags == 0) {
            // unsolo
            mTrackFlags = ModuleDocument::AllOn;
        } else {
            // solo
            mTrackFlags = (ModuleDocument::OutputFlag)(1 << mTrackHover);
        }
        mDocument->setChannelOutput(mTrackFlags);

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
        mTrackFlags ^= (ModuleDocument::OutputFlag)(1 << mTrackHover);
        mDocument->setChannelOutput(mTrackFlags);
        update();
    }
}

void PatternGridHeader::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        update();
    }
}

void PatternGridHeader::setOutputFlags(ModuleDocument::OutputFlags flags) {
    if (mTrackFlags != flags) {
        mTrackFlags = flags;
        update();
    }
}
