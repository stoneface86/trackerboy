
#include "widgets/sidebar/AudioScope.hpp"

#include <QGuiApplication>
#include <QMutexLocker>
#include <QPainter>
#include <QPen>

#include <QtDebug>

#include <cmath>
#include <cstdlib>

constexpr int LINE_WIDTH = 1;


AudioScope::AudioScope(QWidget *parent) :
    QFrame(parent),
    mBuffer(nullptr)
{
    setAttribute(Qt::WA_StyledBackground);
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

    setFrameStyle(QFrame::Box | QFrame::Plain);
    
    setLineWidth(LINE_WIDTH);
    setFixedHeight(WAVE_HEIGHT * 2 + LINE_WIDTH * 2);

}

void AudioScope::setBuffer(Guarded<VisualizerBuffer> *buffer) {
    if (buffer != mBuffer) {
        mBuffer = buffer;
        update();
    }
}

void AudioScope::paintEvent(QPaintEvent *evt) {
    QFrame::paintEvent(evt);

    // it may be more efficient to do renderering in a separate thread
    // we can convert a sample buffer to a buffer of QLines, then
    // the paint event just draws the lines

    if (mBuffer == nullptr) {
        // no buffer, draw nothing
        drawSilence();
        return;
    }

    auto handle = mBuffer->access();
    auto size = handle->size();
    if (size == 0) {
        // buffer is empty, draw nothing
        drawSilence();
        return;
    }

    auto const w = width() - (LINE_WIDTH * 2);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(palette().color(QPalette::WindowText));

    // pixels per sample
    float ratio = (unsigned)size / (float)w;
    float index = 0;

    float prevLeft;
    float prevRight;
    sample(handle, 0.0f, ratio, prevLeft, prevRight);

    int const end = w + LINE_WIDTH;
    for (int t = 1 + LINE_WIDTH; t < end; ++t) {
        index += ratio;
        
        float leftSample;
        float rightSample;
        sample(handle, index, ratio, leftSample, rightSample);

        painter.drawLine(QLineF(t - 1, prevLeft, t, leftSample));
        painter.drawLine(QLineF(t - 1, prevRight, t, rightSample));

        prevLeft = leftSample;
        prevRight = rightSample;

        
    }
    
}

void AudioScope::drawSilence() {
    QPainter painter(this);
    painter.setPen(palette().color(QPalette::WindowText));
    auto const x2 = width() - LINE_WIDTH;
    painter.drawLine(LINE_WIDTH, WAVE_LEFT_AXIS, x2, WAVE_LEFT_AXIS);
    painter.drawLine(LINE_WIDTH, WAVE_RIGHT_AXIS, x2, WAVE_RIGHT_AXIS);

}

void AudioScope::sample(Locked<VisualizerBuffer> &handle, float index, float ratio, float &outLeft, float &outRight) {
    float left, right;
    handle->averageSample(index, ratio, left, right);

    outLeft = WAVE_LEFT_AXIS - (left / (65536.0f / WAVE_HEIGHT));
    outRight = WAVE_RIGHT_AXIS - (right / (65536.0f / WAVE_HEIGHT));
}
