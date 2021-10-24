
#include "widgets/GraphEdit.hpp"

#include "trackerboy/note.hpp"

#include <QtDebug>

#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

#define TU GraphEditTU
namespace TU {

constexpr int MIN_CELL_WIDTH = 12;
constexpr int MAX_CELL_WIDTH = 32;

constexpr int MIN_CELL_HEIGHT = 8;

constexpr int PADDING_X = 8;
constexpr int PADDING_Y = 8;


//
// special modulo - idk what to call this
// mod(-3, 3) = 0
// mod(-2, 3) = 2
// mod(-1, 3) = 1
// mod( 0, 3) = 0
// mod( 1, 3) = 2
// mod( 2, 3) = 1
// mod( 3, 3) = 0
//
constexpr int mod(int a, int b) {
    auto rem = a % b;
    return rem < 0 ? -rem : (b - rem) % b;
}


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

}


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
    mAlternateInterval(0),
    mMouseOver()
{
    auto _viewport = viewport();
    _viewport->setMouseTracking(true);
    mPlotRect = _viewport->rect();
    mPlotRect.adjust(TU::PADDING_X, TU::PADDING_Y, -TU::PADDING_X, -TU::PADDING_Y);

    connect(&model, &GraphModel::dataChanged, _viewport, qOverload<>(&QWidget::update));
    connect(&model, &GraphModel::countChanged, this,
        [this](int count) {
            Q_UNUSED(count)
            calculateCellWidth();
            viewport()->update();
        });

    calculateAxis();
    setViewModeImpl(mMode);
}

GraphEdit::ViewMode GraphEdit::viewMode() const {
    return mMode;
}

int GraphEdit::minimumValue() const {
    return mMinValue;
}

int GraphEdit::maximumValue() const {
    return mMaxValue;
}

void GraphEdit::setColors(Palette const& pal) {
    mBackgroundColor = pal[Palette::ColorGraphBackground];
    mAlternateColor = pal[Palette::ColorGraphAlternate];
    mSampleColor = pal[Palette::ColorGraphSamples];
    mLineColor = pal[Palette::ColorGraphLines];

    viewport()->update();
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
            mAlternateInterval = 12; // highlight every octave row
            break;
        case TimbreView:
        case PanningView:
            mBarMode = true;
            mLines = true;
            mMinValue = 0;
            mMaxValue = 3;
            mAlternateInterval = 0;
            break;
        case PitchView:
            mBarMode = true;
            mLines = false;
            mMinValue = -128;
            mMaxValue = 127;
            mAlternateInterval = 0; // no highlights
            break;
        case WaveformView:
            mBarMode = false;
            mMinValue = 0;
            mMaxValue = 0xF;
            mAlternateInterval = 2;
            break;
    }
    calculateCellHeight();
    calculateCellWidth();

    auto vscroll = verticalScrollBar();
    auto vmax = vscroll->maximum();
    auto vmin = vscroll->minimum();
    if (vmin != vmax) {
        // center the scroll position
        auto pageStep = vscroll->pageStep();
        auto range = vmax + 1 + pageStep - vmin;
        auto value = (range - vscroll->pageStep()) / 2 + vmin;
        vscroll->setValue(value);
    }

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

    // doing this manually instead of using auto-fill background because changing the
    // background role color in QPalette has no effect (tried both this widgets and the viewport widgets palette)
    painter.fillRect(_viewport->rect(), mBackgroundColor);

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
    painter.drawText(TU::PADDING_X, mPlotRect.top(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignTop, QString::number(viewportMax));
    painter.drawText(TU::PADDING_X, _viewport->height() - TU::PADDING_Y - metrics.height(), mYAxisWidth, metrics.height(), Qt::AlignRight | Qt::AlignBottom, QString::number(viewportMin));


    // Grid lines
    painter.setPen(mLineColor);
    auto const plotTop = mPlotRect.top() - 1;
    auto const plotBottom = mPlotRect.bottom();
    auto const axis = mPlotRect.left() - 1;
    auto const viewportWidth = _viewport->width();
    painter.drawLine(axis, plotTop, axis, plotBottom);
    painter.drawLine(axis + 1, plotTop, viewportWidth, plotTop);
    painter.drawLine(axis + 1, plotBottom, viewportWidth, plotBottom);

    if (mBarMode) {
        TU::BarPlotter plotter(mPlotRect, mMinValue, mMaxValue);
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

        if (mAlternateInterval) {
            int ypos = mPlotRect.bottom() - TU::mod(viewportMin, mAlternateInterval) * mCellHeight;
            int skip = mAlternateInterval * mCellHeight;
            for (int const top = mPlotRect.top(); ypos > top; ypos -= skip) {
                painter.fillRect(mPlotRect.left(), ypos - mCellHeight + 1, viewportWidth, mCellHeight - 1, mAlternateColor);
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
    mPlotRect.setLeft(mYAxisWidth + (TU::PADDING_X * 2));
    mPlotRect.setRight(viewport()->width() - TU::PADDING_X);
    calculateCellWidth();
}

int GraphEdit::availableWidth() {
    return viewport()->width() - mPlotRect.left();
}

int GraphEdit::availableHeight() {
    return viewport()->height() - TU::PADDING_Y * 2;
}

void GraphEdit::calculateCellWidth() {
    // determine cell width
    auto const _width = availableWidth();
    auto count = mModel.count();
    if (count) {
        auto cellWidth = _width / count;
        if (cellWidth < TU::MIN_CELL_WIDTH) {
            // horizontal scrollbar is needed
            cellWidth = TU::MIN_CELL_WIDTH;
            auto pagestep = _width / TU::MIN_CELL_WIDTH;
            auto scrollbar = horizontalScrollBar();
            scrollbar->setPageStep(pagestep);
            scrollbar->setMaximum(count - pagestep - 1);
            mPlotRect.setWidth(pagestep * TU::MIN_CELL_WIDTH + 1);
        } else {
            cellWidth = std::min(cellWidth, TU::MAX_CELL_WIDTH);
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
            if (cellHeight < TU::MIN_CELL_HEIGHT) {
                cellHeight = TU::MIN_CELL_HEIGHT;
                // idk why the -1 is needed but it works
                auto pagestep = (_height / TU::MIN_CELL_HEIGHT) - 1;
                auto scrollbar = verticalScrollBar();

                // keep the current value proportional to the new range
                auto value = scrollbar->value() + (scrollbar->pageStep() + 1) / 2 - (pagestep + 1) / 2;

                scrollbar->setPageStep(pagestep);
                scrollbar->setRange(mMinValue, mMaxValue - pagestep);
                scrollbar->setValue(value);
                newHeight = (pagestep + 1) * TU::MIN_CELL_HEIGHT;
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

#undef TU
