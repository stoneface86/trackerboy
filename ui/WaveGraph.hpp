
#pragma once

#include <QFrame>
#include <QMouseEvent>
#include <QPainter>


class WaveGraph : public QFrame {

    Q_OBJECT

public:
    WaveGraph(QWidget *parent = nullptr);

    void setData(uint8_t *data);

signals:
    // when the mouse is within the widget, this signal is emitted whenever
    // the current coordinates the mouse points to changes. The string emitted
    // is in the format "(x, y)" where x is the sample index and y is the sample value
    void coordsTextChanged(const QString &string);

    //
    // emitted when the user changes a sample via clicking on the plot. The new sample value
    // is stored in a QPoint's y member and the corresponding index in the x member.
    //
    void sampleChanged(QPoint sample);

protected:

    void paintEvent(QPaintEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;
    void leaveEvent(QEvent *evt) override;

    void resizeEvent(QResizeEvent *evt) override;


private:

    // resize the plot rectangle
    void calcGraph();
    
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
    
    // pointer to the sample data (array of 32 bytes, stored in the WaveEditor class)
    uint8_t *mData;



};
