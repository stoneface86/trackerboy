
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

constexpr int MIN_CELL_HEIGHT = 8;

constexpr int PADDING_X = 8;
constexpr int PADDING_Y = 8;

class BarPlotter {

    int const mRange;
    int const mMin;
    int const mHeight;
    int const mBottom;
    int mXAxis;

public:
    BarPlotter(QRect const& rect, int min, int max) :
        mRange(abs(max - min)),
        mMin(min),
        mHeight(rect.height()),
        mBottom(rect.bottom()),
        mXAxis(0)
    {
        int axis = std::max(0, min);
        mXAxis = plot((GraphModel::DataType)axis);
        
    }

    int xaxis() const {
        return mXAxis;
    }

    int plot(GraphModel::DataType input) {
        return mBottom - ((mHeight * ((int)input - mMin)) / mRange);
    }

};


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
    connect(&model, &GraphModel::countChanged, this,
        [this](int count) {
            Q_UNUSED(count)
            calculateCellWidth();
            update();
        });

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
            mMinValue = -128;
            mMaxValue = 127;
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
            mMinValue = 0;
            mMaxValue = 0xF;
            break;
    }
    calculateCellHeight();
    calculateCellWidth();

    auto vscroll = verticalScrollBar();
    auto vmax = vscroll->maximum();
    auto vmin = vscroll->minimum();
    if (vmin != vmax) {
        // center the scroll position
        auto range = vmax + 1 - vmin;
        auto value = (range - vscroll->pageStep()) / 2 + vmin;
        vscroll->setValue(value);
    }

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

    // paint on the viewport widget
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

    auto metrics = fontMetrics();

    // viewport minimum/maximum or just minimum/maximum
    painter.setPen(mSampleColor);
    painter.drawText(PADDING_X, mPlotRect.top(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignTop, QString::number(viewportMax));
    painter.drawText(PADDING_X, _viewport->height() - PADDING_Y - metrics.height(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignBottom, QString::number(viewportMin));


    // Grid lines
    painter.setPen(mLineColor);
    auto const plotTop = mPlotRect.top() - 1;
    auto const plotBottom = mPlotRect.bottom() + 1;
    auto const axis = mPlotRect.left() - 1;
    auto const viewportWidth = _viewport->width();
    painter.drawLine(axis, plotTop, axis, plotBottom);
    painter.drawLine(axis + 1, plotTop, viewportWidth, plotTop);
    painter.drawLine(axis + 1, plotBottom, viewportWidth, plotBottom);

    if (mBarMode) {
        BarPlotter plotter(mPlotRect, mMinValue, mMaxValue);
        if (mLines) {
            for (int i = mMinValue + 1; i < mMaxValue; ++i) {
                auto ypos = plotter.plot((GraphModel::DataType)i);
                painter.drawLine(mPlotRect.left(), ypos, viewportWidth, ypos);
            }
        }

        // xaxis (only drawn if the minimum value isn't 0, since then it is the bottom line of the plot)
        if (mMinValue) {
            painter.drawLine(mPlotRect.left(), plotter.xaxis(), viewportWidth, plotter.xaxis());
        }

        // mouse hover
        painter.translate(QPoint(mPlotRect.x(), 0));
        painter.setPen(mSampleColor);
        if (mMouseOver) {
            painter.drawRect(
                mMouseOver->x() * mCellWidth,
                plotter.xaxis(), 
                mCellWidth, 
                plotter.plot((GraphModel::DataType)mMouseOver->y()) - plotter.xaxis()
            );
        }

        // bars
        auto count = mModel.count();
        int xpos = 1;
        for (auto i = hscroll->value(); i < count; ++i, xpos += mCellWidth) {
            auto h = plotter.plot(mModel.dataAt(i)) - plotter.xaxis();
            if (h == 0) {
                h = 1;
            }
            painter.fillRect(xpos, plotter.xaxis(), mCellWidth - 1, h, mSampleColor);
        }

    } else {
        // always draw lines for sample view
        {
            int ypos = mPlotRect.bottom() - mCellHeight;
            for (int const top = mPlotRect.top(); ypos > top; ypos -= mCellHeight) {
                painter.drawLine(mPlotRect.left(), ypos, viewportWidth, ypos);
            }
        }

        painter.translate(QPoint(mPlotRect.x(), mPlotRect.bottom() + viewportMin * mCellHeight));
        painter.scale(1.0, -1.0);
        painter.setPen(mSampleColor);
        
        if (mMouseOver) {
            painter.drawRect(
                mMouseOver->x() * mCellWidth,
                (mMouseOver->y() + viewportMin) * mCellHeight, 
                mCellWidth, 
                mCellHeight
            );
        }

        auto count = mModel.count();
        int xpos = 1;
        for (int i = hscroll->value(); i < count; ++i, xpos += mCellWidth) {
            auto sample = mModel.dataAt(i);
            if (sample < viewportMin || sample > viewportMax) {
                continue;
            }
            auto ypos = sample * mCellHeight;
            QRect rect(xpos, ypos, mCellWidth, mCellHeight);
            painter.fillRect(xpos, ypos, mCellWidth - 1, mCellHeight - 1, mSampleColor);
        }
    }

}

void GraphEdit::mouseMoveEvent(QMouseEvent *evt) {
    if (mModel.count() == 0) {
        return;
    }

    auto pos = evt->pos();
    if (evt->buttons() & Qt::LeftButton) {
        auto coords = mouseToPlotCoordinates(pos);
        if (mLastMouseCoords != coords) {
            mLastMouseCoords = coords;
            setDataAtMouse(coords);
        }
    } else {
        updateHover(pos);
    }

}

void GraphEdit::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() != Qt::LeftButton || mModel.count() == 0) {
        return;
    }

    if (mMouseOver) {
        mMouseOver.reset();
        viewport()->update();
    }

    mLastMouseCoords = mouseToPlotCoordinates(evt->pos());
    setDataAtMouse(mLastMouseCoords);
}

void GraphEdit::mouseReleaseEvent(QMouseEvent *evt) {
    if (evt->button() != Qt::LeftButton || mModel.count() == 0) {
        return;
    }

    updateHover(evt->pos());
}

void GraphEdit::resizeEvent(QResizeEvent *evt) {

    auto oldsize = evt->oldSize();
    auto newsize = evt->size();

    if (oldsize.width() != newsize.width()) {
        calculateCellWidth();
    }

    if (oldsize.height() != newsize.height()) {
        calculateCellHeight();
    }

}

void GraphEdit::setDataAtMouse(QPoint coords) {
    GraphModel::DataType y;
    if (mBarMode) {
        y = (GraphModel::DataType)coords.y();
    } else {
        auto vscroll = verticalScrollBar();
        if (vscroll->isVisible()) {
            y = (GraphModel::DataType)(vscroll->maximum() - (vscroll->value() - vscroll->minimum()) + coords.y());
        } else {
            y = (GraphModel::DataType)coords.y();
        }
    }

    mModel.setData(
        horizontalScrollBar()->value() + coords.x(),
        y
    );
}

QPoint GraphEdit::mouseToPlotCoordinates(QPoint mouse) {

    int x;
    if (mouse.x() >= mPlotRect.right()) {
        x = (mPlotRect.width() / mCellWidth) - 1;
    } else if (mouse.x() < mPlotRect.left()) {
        x = 0;
    } else {
        x = (mouse.x() - mPlotRect.left()) / mCellWidth;
    }

    auto y = mPlotRect.bottom() - mouse.y();
    bool const belowBottom = y < 0;
    bool const aboveTop = mPlotRect.top() >= mouse.y();
    
    if (mBarMode) {
        if (belowBottom) {
            y = mMinValue;
        } else if (aboveTop) {
            y = mMaxValue;
        } else {
            auto range = mMaxValue - mMinValue;
            y += mPlotRect.height() / range / 2;
            y = y * range / mPlotRect.height() + mMinValue;
            y = std::clamp(y, mMinValue, mMaxValue);
        }
    } else {
        if (belowBottom) {
            y = 0;
        } else if (aboveTop) {
            y = (mPlotRect.height() - 1) / mCellHeight;
        } else {
            y = y / mCellHeight;
        }
    }

    return {x, y};
}

void GraphEdit::updateHover(QPoint mouse) {
    if (mPlotRect.contains(mouse, true)) {
        auto mousePos = mouseToPlotCoordinates(mouse);
        if (mMouseOver != mousePos) {
            mMouseOver = mousePos;
            viewport()->update();
        }
    } else if (mMouseOver) {
        mMouseOver.reset();
        viewport()->update();
    }
}

void GraphEdit::calculateAxis() {

    // PADDING + mYAxisWidth + PADDING + mPlotRect.width() + PADDING = viewport()->width()
    auto metrics = fontMetrics();
    mYAxisWidth = metrics.averageCharWidth() * 4;
    mPlotRect.setLeft(mYAxisWidth + (PADDING_X * 2));
    mPlotRect.setRight(viewport()->width() - PADDING_X);
    calculateCellWidth();
}

int GraphEdit::availableWidth() {
    return viewport()->width() - mPlotRect.left();
}

int GraphEdit::availableHeight() {
    return viewport()->height() - PADDING_Y * 2;
}

void GraphEdit::calculateCellWidth() {
    // determine cell width
    auto const _width = availableWidth();
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
    auto const _height = availableHeight();

    auto range = abs(mMaxValue + 1 - mMinValue);
    if (range) {
        int newHeight;
        auto cellHeight = _height / range;
        if (mBarMode) {
            verticalScrollBar()->setRange(0, 0);
            newHeight = _height;
        } else {
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
        }
        
        mPlotRect.setTop((viewport()->height() - newHeight) / 2);
        mPlotRect.setHeight(newHeight);
        
        if (cellHeight == 0) {
            cellHeight = 1;
        }
        mCellHeight = cellHeight;
    }
}
