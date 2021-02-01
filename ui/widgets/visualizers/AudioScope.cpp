
#include "widgets/visualizers/AudioScope.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QPen>

#include <QtDebug>

#include <cmath>
#include <cstdlib>

constexpr int LINE_WIDTH = 1;

AudioScope::AudioScope(QWidget *parent) :
    QFrame(parent),
    mDuration(2400),
    mBinSize(1),
    mPreviousBin(0.0f),
    mWaveform(WAVE_WIDTH, WAVE_HEIGHT)
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

    setFrameStyle(QFrame::Box | QFrame::Plain);
    
    setLineWidth(LINE_WIDTH);
    setFixedSize(WAVE_WIDTH + LINE_WIDTH * 2, WAVE_HEIGHT + LINE_WIDTH * 2);

    clear();
}

void AudioScope::clear() {
    auto const w = width();

    auto &bgcolor = palette().color(QPalette::Window);
    mWaveform.fill(bgcolor);

    QPainter painter(&mWaveform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawLine(0, WAVE_AXIS, WAVE_WIDTH, WAVE_AXIS);

    
    mPreviousBin = 0.0f;
    
    update();
}

void AudioScope::setDuration(size_t samples) {
    if (mDuration != samples) {
        mDuration = samples;
        mBinSize = std::max((size_t)1, mDuration / (size_t)WAVE_WIDTH);
    }
}

void AudioScope::render(int16_t samples[], size_t nsamples, size_t skip) {
    Q_UNUSED(nsamples);

    auto &pal = palette();
    mWaveform.fill(pal.color(QPalette::Window));

    QPainter painter(&mWaveform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(QPoint(0, WAVE_HEIGHT / 2 - 1));
    painter.scale(1.0, -1.0);
    painter.setPen(pal.color(QPalette::WindowText));

    // previous sample from the previous bin
    float prev = mPreviousBin;
    auto dataPtr = samples;

    for (int t = 0; t != WAVE_WIDTH; ++t) {

        // sample the current bin (averaging)
        float sample = 0.0f;
        for (int i = 0; i != mBinSize; ++i) {
            sample += *dataPtr / (65536.0f / WAVE_HEIGHT);
            dataPtr += skip;
        }
        sample /= mBinSize;

        painter.drawLine(QLineF(t - 1, prev, t, sample));

        prev = sample;
    }

    mPreviousBin = prev;

    painter.end();
}

void AudioScope::paintEvent(QPaintEvent *evt) {
    QFrame::paintEvent(evt);

    QPainter painter(this);
    painter.drawPixmap(LINE_WIDTH, LINE_WIDTH, mWaveform);
    
}
