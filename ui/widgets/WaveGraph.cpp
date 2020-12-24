
#include "widgets/WaveGraph.hpp"

#include <QPainter>



constexpr int PADDING_X = 4;
constexpr int PADDING_Y = 4;

constexpr int MIN_WIDTH = 8 * 32 + (PADDING_X * 2);
constexpr int MIN_HEIGHT = 12 * 16 + (PADDING_Y * 2);

WaveGraph::WaveGraph(WaveListModel &model, QWidget *parent) :
    mModel(model),
    mDragging(false),
    mCurX(0),
    mCurY(0),
    mPlotAxisColor(0x8E, 0x8E, 0x8E),
    mPlotGridColor(0x20, 0x20, 0x20),
    mPlotLineColor(0xC0, 0xC0, 0xC0),
    mPlotSampleColor(0xFF, 0xFF, 0xFF),
    mPlotRect(),
    mCellWidth(0),
    mCellHeight(0),
    mMouseOver(false),
    mData{ 0 },
    QFrame(parent)
{
    calcGraph();
    setMouseTracking(true);
    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    // black background
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    connect(&model, &WaveListModel::currentIndexChanged, this,
        [this](int index) {
            if (index != -1) {
                waveformUpdated();
            }
        });
    connect(&model, qOverload<>(&WaveListModel::waveformChanged), this, &WaveGraph::waveformUpdated);
}


void WaveGraph::paintEvent(QPaintEvent *evt) {
    (void)evt;

    QPainter painter(this);

    // transform to make painting easier
    painter.translate(QPoint(mPlotRect.x(), height() - mPlotRect.y()));
    painter.scale(1.0, -1.0);

    int const cPlotwidth = mPlotRect.width();

    // axis
    painter.setPen(mPlotAxisColor);
    painter.drawLine(0, 0, cPlotwidth, 0);

    // lines
    painter.setPen(mPlotGridColor);
    int yline = 0;
    for (int i = 0; i != 15; ++i) {
        yline += mCellHeight;
        painter.drawLine(0, yline, cPlotwidth, yline);
    }

    painter.setPen(mPlotLineColor);
    
    // cursor
    if (mMouseOver) {
        painter.drawRect(mCurX * mCellWidth, mCurY * mCellHeight, mCellWidth, mCellHeight);
    }

    // samples
    QBrush brush(mPlotSampleColor);
    painter.setBrush(brush);
    
    for (int i = 0, x = 0; i != 32; ++i, x += mCellWidth) {
        painter.drawRect(x, mData[i] * mCellHeight, mCellWidth, mCellHeight);
    }

}

void WaveGraph::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        mDragging = true;
        mData[mCurX] = mCurY;
        mModel.setSample(QPoint(mCurX, mCurY));
        
        update();
    }
}

void WaveGraph::mouseReleaseEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        mDragging = false;
    }
}


void WaveGraph::mouseMoveEvent(QMouseEvent *evt) {

    uint8_t newX, newY;

    auto mx = evt->x();
    auto my = evt->y();

    if (mx < mPlotRect.x()) {
        newX = 0;
    } else if (mx > mPlotRect.right()) {
        newX = 31;
    } else {
        newX = static_cast<uint8_t>((mx - mPlotRect.x()) / mCellWidth);
    }

    if (my < mPlotRect.y()) {
        newY = 0xF;
    } else if (my > mPlotRect.bottom()) {
        newY = 0;
    } else {
        newY = 0xF - static_cast<uint8_t>((my - mPlotRect.y()) / mCellHeight);
    }

    if (mCurX != newX || mCurY != newY) {
        mCurX = newX;
        mCurY = newY;
        emit coordsTextChanged(QString("(%1, %2)").arg(QString::number(newX), QString::number(newY)));
        if (mDragging) {
            if (mData[mCurX] != mCurY) {
                mData[mCurX] = mCurY;
                mModel.setSample(QPoint(mCurX, mCurY));
                
                
            }
        }
        mMouseOver = true;
        update();
    }

}

void WaveGraph::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);

    emit coordsTextChanged("");
    mMouseOver = false;
    update();
}

void WaveGraph::resizeEvent(QResizeEvent *evt) {
    Q_UNUSED(evt);

    calcGraph();
}

void WaveGraph::waveformUpdated() {
    auto wavedata = mModel.currentWaveform()->data();
    for (int i = 0, j = 0; i != 16; ++i) {
        uint8_t sample = wavedata[i];
        mData[j++] = sample >> 4;
        mData[j++] = sample & 0xF;
    }
    update();
}


void WaveGraph::calcGraph() {

    int cellwidth = (width() - (PADDING_X * 2)) / 32;
    if (mCellWidth != cellwidth) {
        mPlotRect.setWidth(cellwidth * 32);
        mCellWidth = cellwidth;
    }

    int cellheight = (height() - (PADDING_Y * 2)) / 16;
    if (mCellHeight != cellheight) {
        mPlotRect.setHeight(cellheight * 16);
        mCellHeight = cellheight;
    }

    // re-center plot rectangle
    mPlotRect.moveCenter(rect().center());
}


