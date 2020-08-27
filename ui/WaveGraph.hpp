
#pragma once

#include <QFrame>
#include <QMouseEvent>
#include <QPainter>

#include "trackerboy/synth/Waveform.hpp"

class WaveGraph : public QFrame {

    Q_OBJECT

public:
    WaveGraph(QWidget *parent = nullptr);

protected:

    void paintEvent(QPaintEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;


private:

    void calcGraph();
    uint8_t calcSample(int y);
    void setSample(uint8_t sample);

    void drawSample(QPainter &painter, uint8_t sample, float x);

    bool mDragging;
    uint8_t mLastSample;
    int mSampleIndex;
    
    float mStepX;
    float mStepY;

    QColor mPlotAxisColor;
    QColor mPlotGridColor;
    QColor mPlotLineColor;
    QColor mPlotSampleColor;
    
    trackerboy::Waveform mWaveform;



};
