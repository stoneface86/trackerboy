
#include "widgets/config/SoundQualityPreview.hpp"

#include <QPainter>

#include <array>

// Q0.16 (taken from blip_buf.c, phase = 1)
static std::array<int16_t, 16> SINC_SAMPLE = {
    44, -118,  348, -473, 1076, -799, 5274, 21001,
    6464, -1021, 40, -110, 350, -499, 1190, 1

};



SoundQualityPreview::SoundQualityPreview(QWidget *parent) :
    QFrame(parent),
    mQuality(false),
    mPath()
{
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);

    setMinimumSize(64, 64);
}


void SoundQualityPreview::setHighQuality(bool quality) {
    if (mQuality != quality) {
        mQuality = quality;
        updatePath();
        update();
    }
}

void SoundQualityPreview::paintEvent(QPaintEvent *evt) {
    QFrame::paintEvent(evt);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    auto const &pal = palette();
    auto const crect = contentsRect();

    painter.fillRect(crect, pal.brush(QPalette::Base));

    painter.setPen(QPen(pal.text(), 1, Qt::SolidLine, Qt::RoundCap));
    painter.translate(crect.topLeft());
    painter.drawPath(mPath.value());

}

void SoundQualityPreview::resizeEvent(QResizeEvent *evt) {
    Q_UNUSED(evt)
    updatePath();
}

void SoundQualityPreview::updatePath() {
    auto const crect = contentsRect();
    auto const ch = crect.height();
    auto const cw = crect.width();
    auto const startY = 0.75 * ch;
    auto const endY = 0.25 * ch;
    
    // clear was added in 5.13 so we cannot use it
    //mPath.clear();
    // instead we reconstruct a new QPainterPath
    mPath.emplace();
    auto &path = mPath.value();
    path.moveTo(0.0, startY);
    if (mQuality) {
        // draw a sinc-interpolated step
        constexpr double stepWidth = 0.25;
        constexpr double sampleWidth = stepWidth / SINC_SAMPLE.size();

        auto x = (0.5 - (stepWidth / 2)) * cw;
        auto y = startY;
        auto const stepHeight = startY - endY;
        auto const xstep = sampleWidth * cw;
        path.lineTo(x, y);
        for (auto sample : SINC_SAMPLE) {
            x += xstep;
            y -= (sample / 32767.0 * stepHeight);
            path.lineTo(x, y);
        }
    } else {
        // draw a linear-interpolated step
        path.lineTo(0.475 * cw, startY);
        path.lineTo(0.525 * cw, endY);
    }
    path.lineTo(cw, endY);
}
