
#include "widgets/sidebar/AudioScope.hpp"
#include "verdigris/wobjectimpl.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPen>

#define TU AudioScopeTU
namespace TU {

constexpr int LINE_WIDTH = 1;

}

W_OBJECT_IMPL(AudioScope)

AudioScope::AudioScope(QWidget *parent) :
    QFrame(parent),
    mBuffer(nullptr),
    mLineColor(Qt::white)
{
    setAttribute(Qt::WA_StyledBackground);
    setAutoFillBackground(true);

    // Window is the background, default is black
    // WindowText is the line color for the scope, default is white
    auto pal = palette();
    if (pal.isCopyOf(QGuiApplication::palette())) {
        // only modify the palette if we have the default one
        pal.setColor(QPalette::Window, Qt::black);
        setPalette(pal);
    }

    setFrameStyle(QFrame::Box | QFrame::Plain);
    
    setLineWidth(TU::LINE_WIDTH);
    setFixedHeight(WAVE_HEIGHT * 2 + TU::LINE_WIDTH * 2);

}

void AudioScope::setBuffer(Guarded<VisualizerBuffer> *buffer) {
    if (buffer != mBuffer) {
        mBuffer = buffer;
        update();
    }
}

void AudioScope::setColors(Palette const& pal) {
    auto widgetPal = palette();
    widgetPal.setColor(QPalette::Window, pal[Palette::ColorScopeBackground]);
    setPalette(widgetPal);

    mLineColor = pal[Palette::ColorScopeLine];

    update();
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

    auto const w = width() - (TU::LINE_WIDTH * 2);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(mLineColor);

    // pixels per sample
    float ratio = (unsigned)size / (float)w;
    float index = 0;

    float prevLeft;
    float prevRight;
    sample(handle, 0.0f, ratio, prevLeft, prevRight);

    int const end = w + TU::LINE_WIDTH;
    for (int t = 1 + TU::LINE_WIDTH; t < end; ++t) {
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
    auto const x2 = width() - TU::LINE_WIDTH;
    painter.drawLine(TU::LINE_WIDTH, WAVE_LEFT_AXIS, x2, WAVE_LEFT_AXIS);
    painter.drawLine(TU::LINE_WIDTH, WAVE_RIGHT_AXIS, x2, WAVE_RIGHT_AXIS);

}

void AudioScope::sample(Locked<VisualizerBuffer> &handle, float index, float ratio, float &outLeft, float &outRight) {
    float left, right;
    handle->averageSample(index, ratio, left, right);

    outLeft = WAVE_LEFT_AXIS - (left / (2.0f / WAVE_HEIGHT));
    outRight = WAVE_RIGHT_AXIS - (right / (2.0f / WAVE_HEIGHT));
}

#undef TU
