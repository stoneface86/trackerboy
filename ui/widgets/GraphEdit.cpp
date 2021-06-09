
#include "widgets/GraphEdit.hpp"

#include "trackerboy/note.hpp"

#include <QtDebug>

#include <QGuiApplication>
#include <QFontDatabase>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

constexpr int MIN_CELL_WIDTH = 12;
constexpr int MAX_CELL_WIDTH = 32;

constexpr int MIN_CELL_HEIGHT = 6;

constexpr int PADDING_X = 4;
constexpr int PADDING_Y = 4;

// TIP: always call viewport()->update() instead of update()!


GraphEdit::GraphEdit(GraphModel &model, QWidget *parent) :
    QAbstractScrollArea(parent),
    mModel(model),
    mMode(GraphEdit::WaveformView),
    mBarMode(false),
    mLines(true),
    mMinValue(0),
    mMaxValue(0),
    mCellWidth(0),
    mCellHeight(0),
    mMouseOver(),
    mLineColor(0x40, 0x40, 0x40),
    mSampleColor(0xE0, 0xE0, 0xE0)
{
    

    auto _viewport = viewport();
    _viewport->setMouseTracking(true);
    mPlotRect = _viewport->rect();
    mPlotRect.adjust(PADDING_X, PADDING_Y, -PADDING_X, -PADDING_Y);

    connect(&model, &GraphModel::dataChanged, _viewport, qOverload<>(&QWidget::update));
    connect(&model, &GraphModel::countChanged, this, &GraphEdit::calculateCellWidth);

    _viewport->setAttribute(Qt::WA_StyledBackground);

    calculateAxis();
    setViewModeImpl(mMode);
}

QColor GraphEdit::lineColor() const {
    return mLineColor;
}

QColor GraphEdit::sampleColor() const {
    return mSampleColor;
}

GraphEdit::ViewMode GraphEdit::viewMode() const {
    return mMode;
}

void GraphEdit::setViewMode(ViewMode mode) {
    if (mMode != mode) {
        setViewModeImpl(mode);
    }
}

void GraphEdit::setViewModeImpl(ViewMode mode) {
    mMode = mode;
    switch (mode) {
        case ArpeggioView:
            mBarMode = false;
            mLines = true;
            mMinValue = 0;
            mMaxValue = trackerboy::NOTE_LAST;
            break;
        case TimbreView:
        case PanningView:
            mBarMode = true;
            mLines = true;
            mMinValue = 0;
            mMaxValue = 3;
            break;
        case PitchView:
            mBarMode = true;
            mLines = false;
            mMinValue = -128;
            mMaxValue = 127;
            break;
        case WaveformView:
            mBarMode = false;
            mLines = true;
            mMinValue = 0;
            mMaxValue = 0xF;
            break;
    }
    calculateCellHeight();
    calculateCellWidth();
    update();
}

void GraphEdit::setLineColor(QColor color) {
    mLineColor = color;
    viewport()->update();
}

void GraphEdit::setSampleColor(QColor color) {
    mSampleColor = color;
    viewport()->update();
}

void GraphEdit::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt)

    if (mMouseOver) {
        mMouseOver.reset();
        viewport()->update();
    }
}

void GraphEdit::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt)

    auto _viewport = viewport();
    QPainter painter(_viewport);

    auto hscroll = horizontalScrollBar();
    auto vscroll = verticalScrollBar();
    
    int viewportMin;
    int viewportMax;
    if (vscroll->minimum() == vscroll->maximum()) {
        viewportMin = mMinValue;
        viewportMax = mMaxValue;
    } else {
        auto value = vscroll->maximum() - (vscroll->value() - vscroll->minimum());
        viewportMin = value;
        viewportMax = value + vscroll->pageStep();
    }

    // Grid lines
    painter.setPen(mLineColor);
    auto gridRect = mPlotRect.adjusted(-1, -1, 0, 0);
    painter.drawRect(gridRect);
    if (mLines) {
        if (mBarMode) {

        } else {
            int ypos = mPlotRect.bottom() - mCellHeight;
            for (int const top = mPlotRect.top(); ypos > top; ypos -= mCellHeight) {
                painter.drawLine(mPlotRect.left(), ypos, mPlotRect.right(), ypos);
            }
        }
    }

    auto metrics = fontMetrics();

    painter.setPen(mSampleColor);
    painter.drawText(PADDING_X, mPlotRect.top(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignTop, QString::number(viewportMax));
    painter.drawText(PADDING_X, _viewport->height() - PADDING_Y - metrics.height(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignBottom, QString::number(viewportMin));

    //painter.fillRect(mPlotRect, Qt::gray);

    painter.translate(QPoint(mPlotRect.x(), mPlotRect.bottom() + viewportMin * mCellHeight));
    painter.scale(1.0, -1.0);

    if (mBarMode) {

    } else {
        if (mMouseOver) {
            painter.drawRect(
                mMouseOver->x() * mCellWidth,
                (mMouseOver->y() + viewportMin) * mCellHeight, 
                mCellWidth, 
                mCellHeight
            );
        }

        auto count = mModel.count();
        auto color = mSampleColor;
        color.setAlpha(200);
        painter.setPen(mSampleColor);
        painter.setBrush(QBrush(color));
        int xpos = 0;
        for (int i = hscroll->value(); i < count; ++i, xpos += mCellWidth) {
            auto sample = mModel.dataAt(i);
            if (sample < viewportMin || sample > viewportMax) {
                continue;
            }
            auto ypos = sample * mCellHeight;
            QRect rect(xpos, ypos, mCellWidth, mCellHeight);
            painter.drawRect(rect);
        }

    }

}

void GraphEdit::mouseMoveEvent(QMouseEvent *evt) {
    if (mModel.count() == 0) {
        return;
    }

    auto pos = evt->pos();
    if (mPlotRect.contains(pos, true)) {
        QPoint mousePos;

        mousePos.setX((pos.x() - mPlotRect.left()) / mCellWidth);
        if (mBarMode) {
            auto range = abs(mMaxValue - mMinValue);
            auto yInRange = (pos.y() - mPlotRect.top()) * range / mPlotRect.height();
            mousePos.setY(yInRange + mMinValue);
        } else {
            mousePos.setY((mPlotRect.bottom() - pos.y()) / mCellHeight);
        }

        if (!mMouseOver.has_value() || *mMouseOver != mousePos) {
            mMouseOver = mousePos;
            if (evt->buttons() & Qt::LeftButton) {
                setDataAtMouse();
            }
            viewport()->update();
        }
    } else {
        if (mMouseOver) {
            mMouseOver.reset();
            viewport()->update();
        }
    }
}

void GraphEdit::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() != Qt::LeftButton || !mMouseOver) {
        return;
    }

    setDataAtMouse();
}

void GraphEdit::resizeEvent(QResizeEvent *evt) {

    auto oldsize = evt->oldSize();
    auto newsize = evt->size();

    if (oldsize.width() != newsize.width()) {
        mPlotRect.setRight(newsize.width() - PADDING_X);
        calculateCellWidth();
    }

    if (oldsize.height() != newsize.height()) {
        mPlotRect.setTop(PADDING_Y);
        mPlotRect.setBottom(newsize.height() - PADDING_Y);
        calculateCellHeight();
    }

}

void GraphEdit::setDataAtMouse() {
    mModel.setData(
        horizontalScrollBar()->value() + mMouseOver->x(),
        mMouseOver->y()
    );
}

void GraphEdit::recalculate() {
    calculateCellWidth();
    calculateCellHeight();
}

void GraphEdit::calculateAxis() {

    // PADDING + mYAxisWidth + PADDING + mPlotRect.width() + PADDING = viewport()->width()
    auto metrics = fontMetrics();
    mYAxisWidth = metrics.averageCharWidth() * 4;
    mPlotRect.setLeft(mYAxisWidth + (PADDING_X * 2));
    mPlotRect.setRight(viewport()->width() - PADDING_X);
    calculateCellWidth();
}

void GraphEdit::calculateCellWidth() {
    // determine cell width
    auto const _width = mPlotRect.width();
    auto count = mModel.count();
    if (count) {
        auto cellWidth = _width / count;
        if (cellWidth < MIN_CELL_WIDTH) {
            // horizontal scrollbar is needed
            cellWidth = MIN_CELL_WIDTH;
            auto pagestep = _width / MIN_CELL_WIDTH;
            auto scrollbar = horizontalScrollBar();
            scrollbar->setPageStep(pagestep);
            scrollbar->setMaximum(count - pagestep - 1);
            mPlotRect.setWidth(pagestep * MIN_CELL_WIDTH + 1);
        } else {
            cellWidth = std::min(cellWidth, MAX_CELL_WIDTH);
            horizontalScrollBar()->setMaximum(0);
            mPlotRect.setWidth(count * cellWidth + 1);
        }
        mCellWidth = cellWidth;
    }
}

void GraphEdit::calculateCellHeight() {
    auto const _height = mPlotRect.height();

    auto range = abs(mMaxValue + 1 - mMinValue);
    if (range) {
        auto cellHeight = _height / range;
        if (!mBarMode) {
            int newHeight;
            if (cellHeight < MIN_CELL_HEIGHT) {
                cellHeight = MIN_CELL_HEIGHT;
                // idk why the -1 is needed but it works
                auto pagestep = (_height / MIN_CELL_HEIGHT) - 1;
                auto scrollbar = verticalScrollBar();
                scrollbar->setPageStep(pagestep);
                scrollbar->setRange(mMinValue, mMaxValue - pagestep);
                newHeight = (pagestep + 1) * MIN_CELL_HEIGHT;
            } else {
                verticalScrollBar()->setRange(0, 0);
                newHeight = cellHeight * range;
            }
            mPlotRect.setTop(mPlotRect.bottom() - newHeight);
        }

        mCellHeight = cellHeight;
    }
}
