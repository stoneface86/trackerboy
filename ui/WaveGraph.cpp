#include "WaveGraph.hpp"

#include <QPainter>

constexpr int PADDING = 16;

WaveGraph::WaveGraph(QWidget *parent) :
    mDragging(false),
    mLastSample(0),
    mSampleIndex(0),
    mStepX(0.0f),
    mStepY(0.0f),
    mPlotAxisColor(0x8E, 0x8E, 0x8E),
    mPlotGridColor(0x20, 0x20, 0x20),
    mPlotLineColor(0xF0, 0xF0, 0xF0),
    mPlotSampleColor(0xFF, 0xFF, 0xFF),
    QFrame(parent)
{
    mWaveform.fromString("02468ACEEFFFFEEEDDCBA98765432211");
    calcGraph();
}


void WaveGraph::paintEvent(QPaintEvent *evt) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto r = rect();

    painter.fillRect(r, QColorConstants::Black);
    

    float yline = r.height() - PADDING;

    painter.setPen(mPlotAxisColor);
    painter.drawLine(PADDING, yline, r.width() - PADDING, yline);

    painter.setPen(mPlotGridColor);
    for (int i = 0; i != 15; ++i) {
        yline -= mStepY;
        painter.drawLine(PADDING, yline, r.width() - PADDING, yline);
    }

    float x = PADDING;
    for (int i = 0; i != 16; ++i) {
        uint8_t sample = mWaveform[i];

        drawSample(painter, sample >> 4, x);
        x += mStepX;
        drawSample(painter, sample & 0xF, x);
        x += mStepX;
    }

}

void WaveGraph::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        int mx = evt->x();
        if (mx < PADDING || mx > width() - PADDING) {
            return;
        }

        mDragging = true;
        mSampleIndex = (mx - PADDING + (mStepX / 2)) / mStepX;
        mLastSample = calcSample(evt->y());
        setSample(mLastSample);
        repaint();
    }
}

void WaveGraph::mouseReleaseEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        mDragging = false;
    }
}


void WaveGraph::mouseMoveEvent(QMouseEvent *evt) {
    if (mDragging) {
        uint8_t sample = calcSample(evt->y());
        if (sample != mLastSample) {
            setSample(sample);
            mLastSample = sample;
            repaint();
        }
    }
}

void WaveGraph::resizeEvent(QResizeEvent *evt) {
    calcGraph();
    repaint();
}


void WaveGraph::calcGraph() {
    mStepX = (width() - (PADDING * 2)) / 32.0f;
    mStepY = (height() - (PADDING * 2)) / 16.0f;
}

uint8_t WaveGraph::calcSample(int y) {
    y -= PADDING;
    if (y < 0) {
        return 0xF;
    } else if (y > height() - (PADDING * 2)) {
        return 0x0;
    } else {
        return 0xF - static_cast<uint8_t>(y / mStepY);
    }
}

void WaveGraph::setSample(uint8_t sample) {
    int index = mSampleIndex >> 1;
    uint8_t curSample = mWaveform[index];
    if (mSampleIndex & 1) {
        curSample = (curSample & 0xF0) | sample;
    } else {
        curSample = (curSample & 0x0F) | (sample << 4);
    }
    mWaveform[index] = curSample;
}

void WaveGraph::drawSample(QPainter &painter, uint8_t sample, float x) {
    float y = ((16 - sample) * mStepY) + PADDING;
    painter.setPen(mPlotSampleColor);
    painter.drawEllipse(QPointF(x, y), 3.0f, 3.0f);
    painter.setPen(mPlotLineColor);
    painter.drawLine(x, y, x, height() - PADDING);

}
