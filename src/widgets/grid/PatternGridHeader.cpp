
#include "widgets/grid/PatternGridHeader.hpp"

#include <QContextMenuEvent>
#include <QFontDatabase>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QtDebug>

#define TU PatternGridHeaderTU
namespace TU {

constexpr int TRACK_PADDING = 3;

// PM = plus/minus

constexpr int PM_LENGTH = 8; // size of the plus/minus, 8x8
constexpr int PM_THICKNESS = 2; // thickness of each bar
constexpr int PM_CENTER = (PM_LENGTH / 2 - PM_THICKNESS / 2);

constexpr int PM_VPAD = 2; // left and right padding
constexpr int PM_HPAD = 4; // top and bottom padding

constexpr int PM_WIDTH = PM_LENGTH + PM_VPAD * 2;
constexpr int PM_HEIGHT = PM_LENGTH * 2 + PM_HPAD * 2;

constexpr int PM_START_Y = 8;
constexpr int PLUS_START_Y = PM_START_Y;
constexpr int MINUS_START_Y = PM_START_Y + PM_HEIGHT / 2;

void drawPlus(QPainter &painter, int xpos, QColor const& color) {
    // vertical
    painter.fillRect(
        xpos + PM_VPAD + PM_CENTER,
        PLUS_START_Y + PM_HPAD,
        PM_THICKNESS,
        PM_LENGTH,
        color
    );
    // horizontal
    painter.fillRect(
        xpos + PM_VPAD,
        PLUS_START_Y + PM_HPAD + PM_CENTER,
        PM_LENGTH,
        PM_THICKNESS,
        color
    );
}

void drawMinus(QPainter &painter, int xpos, QColor const& color) {
    painter.fillRect(
        xpos + PM_VPAD, 
        MINUS_START_Y + PM_HPAD + PM_CENTER,
        PM_LENGTH,
        PM_THICKNESS,
        color
    );
}

}


PatternGridHeader::PatternGridHeader(PatternModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mLayout(nullptr),
    mTrackHover(HOVER_NONE),
    mHoverKind(HoverKind::track),
    mTrackFlags(ChannelOutput::AllOn),
    mColorBackground(),
    mColorForeground1(),
    mColorForeground2(),
    mColorEnabled(),
    mColorDisabled(),
    mLinePen(),
    mUpdatingFont(false)
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

void PatternGridHeader::toggleTrack(int track) {
    mTrackFlags ^= (ChannelOutput::Flag)(1 << track);
    emit outputChanged(mTrackFlags);
    update();
}

void PatternGridHeader::soloTrack(int track) {
    auto soloFlag = (ChannelOutput::Flag)(1 << track);
    if (soloFlag == mTrackFlags) {
        // unsolo
        mTrackFlags = ChannelOutput::AllOn;
    } else {
        // solo
        mTrackFlags = soloFlag;
    }
    emit outputChanged(mTrackFlags);
    update();
}

void PatternGridHeader::unmuteAll() {
    if (mTrackFlags != ChannelOutput::AllOn) {
        mTrackFlags = ChannelOutput::AllOn;
        emit outputChanged(mTrackFlags);
        update();
    }
}

void PatternGridHeader::changeEvent(QEvent *evt) {
    if (evt->type() == QEvent::FontChange) {
        if (!mUpdatingFont) {
            mUpdatingFont = true;

            // force the set font to have a height of 12 pixels
            auto font_ = font();
            font_.setPixelSize(12);
            setFont(font_);

            mUpdatingFont = false;
        }
    }
}

void PatternGridHeader::contextMenuEvent(QContextMenuEvent *evt) {
    auto menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *act;

    if (mTrackHover != HOVER_NONE) {
        // add actions for the current track
        auto channelNum = mTrackHover + 1;
        auto trackHover = mTrackHover;
        act = menu->addAction(tr("Toggle channel %1").arg(channelNum));
        connect(act, &QAction::triggered, this,
            [this, trackHover]() {
                toggleTrack(trackHover);
            });
        act = menu->addAction(tr("Solo channel %1").arg(channelNum));
        connect(act, &QAction::triggered, this,
            [this, trackHover]() {
                soloTrack(trackHover);
            });
        menu->addSeparator();
    }

    act = menu->addAction(tr("Unmute all channels"));
    connect(act, &QAction::triggered, this, &PatternGridHeader::unmuteAll);

    menu->popup(evt->globalPos());
}

void PatternGridHeader::paintEvent(QPaintEvent *evt) {

    Q_UNUSED(evt);

    if (mLayout == nullptr) {
        return;
    }

    QPainter painter(this);
    QPen textPen(mColorForeground1);

    QColor widgetbg = palette().color(backgroundRole());

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
        auto const contentX = xpos + TU::TRACK_PADDING;
        auto const contentWidth = trackWidth - 5;
        painter.fillRect(contentX, 2, contentWidth, 4, statusColor);

        QColor *plusColor, *minusColor;
        plusColor = minusColor = &mColorForeground1;


        if (mTrackHover == track) {
            painter.fillRect(contentX, h - 2, contentWidth, 1, statusColor);
            switch (mHoverKind) {
                case HoverKind::track:
                    break;
                case HoverKind::addEffect:
                    plusColor = &mColorForeground2;
                    break;
                case HoverKind::removeEffect:
                    minusColor = &mColorForeground2;
                    break;
            }
        }
        
        auto const effectsVisible = mLayout->effectsVisible(track);
        TU::drawPlus(painter, contentX, effectsVisible != 3 ? *plusColor : widgetbg);
        TU::drawMinus(painter, contentX, effectsVisible != 1 ? *minusColor : widgetbg);

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
    if (evt->button() != Qt::LeftButton || mTrackHover == HOVER_NONE) {
        return;
    }

    // the left button was double-clicked, whilest the mouse is over a track
    // and is not over the plus/minus buttons

    switch (mHoverKind) {
        case HoverKind::track:
            if (mTrackFlags == 0) {
                // unsolo
                mTrackFlags = ChannelOutput::AllOn;
            } else {
                // solo
                mTrackFlags = (ChannelOutput::Flag)(1 << mTrackHover);
            }
            emit outputChanged(mTrackFlags);
            update();
            break;
        case HoverKind::addEffect:
            mModel.showEffect(mTrackHover);
            break;
        case HoverKind::removeEffect:
            mModel.hideEffect(mTrackHover);
            break;
    }
    
}

void PatternGridHeader::mouseMoveEvent(QMouseEvent *evt) {

    if (mLayout == nullptr) {
        return;
    }

    setHoverKind(HoverKind::track);
    auto track = mLayout->mouseToTrack(evt->x());
    if (track == 4) {
        track = HOVER_NONE;
    }
    setTrackHover(track);
    if (track != HOVER_NONE) {

        QRect plusMinusRect(
            mLayout->trackToX(track) + TU::TRACK_PADDING,
            TU::PM_START_Y,
            TU::PM_WIDTH,
            TU::PM_HEIGHT
        );
        if (plusMinusRect.contains(evt->pos())) {
            HoverKind kind;
            if (evt->y() < TU::PLUS_START_Y + TU::PM_HEIGHT / 2) {
                kind = HoverKind::addEffect;
            } else {
                kind = HoverKind::removeEffect;
            }
            setHoverKind(kind);
        }
    }
}

void PatternGridHeader::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton && mTrackHover != HOVER_NONE) {
        switch (mHoverKind) {
            case HoverKind::track:
                // user clicked on a track header either to mute or unmute the channel
                toggleTrack(mTrackHover);
                break;
            case HoverKind::addEffect:
                mModel.showEffect(mTrackHover);
                break;
            case HoverKind::removeEffect:
                mModel.hideEffect(mTrackHover);
                break;
        }
    }
}

void PatternGridHeader::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        update();
    }
}

void PatternGridHeader::setHoverKind(HoverKind kind) {
    if (kind != mHoverKind) {
        mHoverKind = kind;
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
