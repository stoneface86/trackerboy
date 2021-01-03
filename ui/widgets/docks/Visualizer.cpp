
#include "Visualizer.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QPen>

#include <QtDebug>

#include <cmath>
#include <cstdlib>

Visualizer::Visualizer(QWidget *parent) :
    QWidget(parent),
    mAntialias(true),
    mDuration(2400),
    mBinSize(1),
    mPreviousBinLeft(0),
    mPreviousBinRight(0),
    mWaveformLeft(),
    mWaveformRight(),
    mOffsetY(0)
{
    setAttribute(Qt::WA_StyledBackground);
    setMinimumHeight(WAVE_HEIGHT * 2);
    setAutoFillBackground(true);

    // Window is the background, default is black
    // WindowText is the line color for the scope, default is white
    auto pal = palette();
    if (pal.isCopyOf(QGuiApplication::palette())) {
        // only modify the palette if we have the default one
        pal.setColor(QPalette::Window, Qt::black);
        pal.setColor(QPalette::WindowText, Qt::white);
        setPalette(pal);
    }

    clear();
}

void Visualizer::clear() {
    clearWaveform(mWaveformLeft);
    clearWaveform(mWaveformRight);

    mPreviousBinLeft = 0;
    mPreviousBinRight = 0;

    update();
}

void Visualizer::setAntialiasing(bool antialias) {
    if (mAntialias != antialias) {
        mAntialias = antialias;
    }
}

void Visualizer::setDuration(int samples) {
    if (mDuration != samples) {
        mDuration = samples;
        mBinSize = std::max(1, mDuration / width());
    }
}


void Visualizer::addSamples(int16_t data[], size_t nsamples) {

    if (nsamples == 0) {
        return;
    }

    int binsToAdd = nsamples / mBinSize;


    int timeIndex;
    

    if (binsToAdd >= mWaveformLeft.width()) {
        // full redraw
        auto &bgcolor = palette().color(QPalette::Window);
        mWaveformLeft.fill(bgcolor);
        mWaveformRight.fill(bgcolor);
        timeIndex = 0;
    } else {
        // partial redraw, just draw the new samples

        timeIndex = mWaveformLeft.width() - binsToAdd;
        scrollWaveform(mWaveformLeft, binsToAdd, timeIndex);
        scrollWaveform(mWaveformRight, binsToAdd, timeIndex);
        
    }

    // draw the waveforms

    QPainter painterLeft(&mWaveformLeft);
    QPainter painterRight(&mWaveformRight);

    initPainter(painterLeft);
    initPainter(painterRight);
    
    auto dataPtr = data;
    auto const w = mWaveformLeft.width();

    // previous sample from the previous bin
    int8_t prevLeft = mPreviousBinLeft;
    int8_t prevRight = mPreviousBinRight;
    int8_t sampleLeft;
    int8_t sampleRight;
    
    for (int t = timeIndex; t != w; ++t) {

        // sample the current bin (averaging)
        
        reduce(dataPtr, sampleLeft, sampleRight, mBinSize);

        painterLeft.drawLine(t - 1, prevLeft, t, sampleLeft);
        painterRight.drawLine(t - 1, prevRight, t, sampleRight);

        prevLeft = sampleLeft;
        prevRight = sampleRight;
    }

    mPreviousBinLeft = prevLeft;
    mPreviousBinRight = prevRight;

    painterLeft.end();
    painterRight.end();

    update();
}

void Visualizer::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt);

    auto const w = width();

    QPainter painter(this);
    // center the scopes
    painter.translate(QPoint(0, mOffsetY));

    // left goes first
    painter.drawPixmap(0, 0, mWaveformLeft);

    // followed by right
    painter.drawPixmap(0, WAVE_HEIGHT, mWaveformRight);

}


void Visualizer::resizeEvent(QResizeEvent *evt) {
    auto const h = height();
    auto const w = width();

    mOffsetY = (h / 2) - (WAVE_HEIGHT);

    mBinSize = std::max(1, mDuration / w);

    if (w != mWaveformLeft.width()) {
        auto &bgcolor = palette().color(QPalette::Window);
        mWaveformLeft = QPixmap(w, WAVE_HEIGHT);
        mWaveformLeft.fill(bgcolor);
        mWaveformRight = QPixmap(w, WAVE_HEIGHT);
        mWaveformRight.fill(bgcolor);
        clear();
    }



}

void Visualizer::clearWaveform(QPixmap &waveform) {
    if (!waveform.isNull()) {
        QPainter painter(&waveform);
        painter.setPen(palette().color(QPalette::WindowText));
        painter.drawLine(0, WAVE_HEIGHT / 2, waveform.width(), WAVE_HEIGHT / 2);

    }
}

void Visualizer::scrollWaveform(QPixmap &waveform, int amount, int timeIndex) {
    auto rect = waveform.rect();
    waveform.scroll(-amount, 0, rect);

    QPainter painter(&waveform);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(timeIndex, 0, rect.width(), rect.height(), palette().color(QPalette::Window));
}

void Visualizer::initPainter(QPainter &painter) {
    if (mAntialias) {
        painter.setRenderHint(QPainter::Antialiasing);
    }
    painter.translate(QPoint(0, WAVE_HEIGHT / 2 - 1));
    painter.scale(1.0, -1.0);
    painter.setPen(palette().color(QPalette::WindowText));
}

void Visualizer::reduce(int16_t *&samples, int8_t &left, int8_t &right, int binSize) {
    // reduce all samples in the bin to the average amplitude
    int sumLeft = 0;
    int sumRight = 0;
    for (int i = 0; i != binSize; ++i) {
        // should we dither? probably fine we won't be hearing this anyways
        sumLeft += static_cast<int8_t>(*samples++ / (65536 / WAVE_HEIGHT));
        sumRight += static_cast<int8_t>(*samples++ / (65536 / WAVE_HEIGHT));
    }
    left = sumLeft / binSize;
    right = sumRight / binSize;
}
