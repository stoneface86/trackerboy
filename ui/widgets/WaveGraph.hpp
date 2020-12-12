
#pragma once

#include <array>

#include <QFrame>
#include <QMouseEvent>
#include <QPainter>

#include "model/WaveListModel.hpp"


class WaveGraph : public QFrame {

    Q_OBJECT

public:
    WaveGraph(WaveListModel &model, QWidget *parent = nullptr);

signals:
    // when the mouse is within the widget, this signal is emitted whenever
    // the current coordinates the mouse points to changes. The string emitted
    // is in the format "(x, y)" where x is the sample index and y is the sample value
    void coordsTextChanged(const QString &string);

protected:

    void paintEvent(QPaintEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;
    void leaveEvent(QEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;

private slots:
    void waveformUpdated();

private:

    // resize the plot rectangle
    void calcGraph();
    
    WaveListModel &mModel;

    // true when the left mouse button is down
    bool mDragging;
    
    // translated mouse coordinates
    uint8_t mCurX; // the sample index
    uint8_t mCurY; // the sample value

    // colors used when painting
    QColor mPlotAxisColor;
    QColor mPlotGridColor;
    QColor mPlotLineColor;
    QColor mPlotSampleColor;

    // boundary of the plot, centered within the widget
    QRect mPlotRect;

    int mCellWidth;
    int mCellHeight;
    bool mMouseOver;
    
    std::array<uint8_t, 32> mData;


};
