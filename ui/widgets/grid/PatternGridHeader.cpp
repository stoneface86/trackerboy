
#include "widgets/grid/PatternGridHeader.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <QMouseEvent>

#define TU PatternGridHeaderTU
namespace TU {

}


PatternGridHeader::PatternGridHeader(QWidget *parent) :
    QWidget(parent),
    mLayout(nullptr),
    mTrackHover(HOVER_NONE),
    mTrackFlags(ChannelOutput::AllOn),
    mColorBackground(),
    mColorForeground1(),
    mColorForeground2(),
    mColorEnabled(),
    mColorDisabled(),
    mLinePen()
{
    setFixedHeight(HEIGHT);
    setAutoFillBackground(true);
    setMouseTracking(true);

    auto _font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    _font.setPixelSize(12);
    setFont(_font);

    mLinePen.setWidth(PatternLayout::LINE_WIDTH);
}

void PatternGridHeader::setColors(Palette const& colorTable) {
    
    auto pal = palette();
    pal.setColor(backgroundRole(), colorTable[Palette::ColorHeaderBackground2]);
    setPalette(pal);
    mColorBackground = colorTable[Palette::ColorHeaderBackground1];
    mColorForeground1 = colorTable[Palette::ColorHeaderForeground1];
    mColorForeground2 = colorTable[Palette::ColorHeaderForeground2];
    mColorEnabled = colorTable[Palette::ColorHeaderEnabled];
    mColorDisabled = colorTable[Palette::ColorHeaderDisabled];
    mLinePen.setColor(colorTable[Palette::ColorLine]);

    update();
}

void PatternGridHeader::setPatternLayout(const PatternLayout *layout) {
    mLayout = layout;
    update();
}


void PatternGridHeader::paintEvent(QPaintEvent *evt) {

    Q_UNUSED(evt);

    if (mLayout == nullptr) {
        return;
    }

    QPainter painter(this);
    QPen textPen(mColorForeground1);

    auto const h = height();

    int xpos = mLayout->patternStart() - PatternLayout::LINE_WIDTH;
    for (int track = 0; track < 4; ++track) {

        auto const trackWidth = mLayout->trackWidth(track);

        painter.setPen(mLinePen);
        painter.drawLine(xpos, 0, xpos, h);
        xpos += PatternLayout::LINE_WIDTH;

        // background color
        painter.fillRect(xpos + 1, 0, trackWidth - 1, h, mColorBackground);

        bool const isPlaying = (mTrackFlags & (1 << track));
        auto const& statusColor = isPlaying ? mColorEnabled : mColorDisabled;
        auto const contentX = xpos + 3;
        auto const contentWidth = trackWidth - 5;
        painter.fillRect(contentX, 2, contentWidth, 4, statusColor);

        if (mTrackHover == track) {
            painter.fillRect(contentX, h - 2, contentWidth, 1, statusColor);
        }

        // text
        painter.setPen(textPen);
        constexpr int textY = 8;
        painter.drawText(contentX, textY, contentWidth, 12, Qt::AlignCenter, QStringLiteral("CH%1").arg(track + 1));

        xpos += trackWidth;
    }

    painter.setPen(mLinePen);
    painter.drawLine(xpos, 0, xpos, h);
    

}

void PatternGridHeader::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);

    setTrackHover(HOVER_NONE);
}

void PatternGridHeader::mouseDoubleClickEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton && mTrackHover != HOVER_NONE) {

        if (mTrackFlags == 0) {
            // unsolo
            mTrackFlags = ChannelOutput::AllOn;
        } else {
            // solo
            mTrackFlags = (ChannelOutput::Flag)(1 << mTrackHover);
        }
        emit outputChanged(mTrackFlags);
        update();
    }
}

void PatternGridHeader::mouseMoveEvent(QMouseEvent *evt) {

    if (mLayout == nullptr) {
        return;
    }

    setTrackHover(mLayout->mouseToTrack(evt->x()));
}

void PatternGridHeader::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton && mTrackHover != HOVER_NONE) {
        // user clicked on a track header either to mute or unmute the channel
        mTrackFlags ^= (ChannelOutput::Flag)(1 << mTrackHover);
        emit outputChanged(mTrackFlags);
        update();
    }
}

void PatternGridHeader::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        update();
    }
}

void PatternGridHeader::setOutputFlags(ChannelOutput::Flags flags) {
    if (mTrackFlags != flags) {
        mTrackFlags = flags;
        emit outputChanged(flags);
        update();
    }
}

#undef TU
