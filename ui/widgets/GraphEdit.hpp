#pragma once

#include "core/model/graph/GraphModel.hpp"

#include <QAbstractScrollArea>
#include <QPoint>

#include <optional>
#include <variant>

class GraphEdit : public QAbstractScrollArea {

    Q_OBJECT
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
    Q_PROPERTY(QColor sampleColor READ sampleColor WRITE setSampleColor)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)

public:

    enum ViewMode {
        ArpeggioView,
        TimbreView,
        PanningView,
        PitchView,
        WaveformView
    };
    Q_ENUM(ViewMode)

    explicit GraphEdit(GraphModel &model, QWidget *parent = nullptr);

    ViewMode viewMode() const;

    QColor lineColor() const;

    QColor sampleColor() const;

    void setLineColor(QColor color);

    void setSampleColor(QColor color);

    void setViewMode(ViewMode mode);


protected:

    virtual void leaveEvent(QEvent *evt) override;

    virtual void paintEvent(QPaintEvent *evt) override;

    virtual void mouseMoveEvent(QMouseEvent *evt) override;

    virtual void mousePressEvent(QMouseEvent *evt) override;

    virtual void mouseReleaseEvent(QMouseEvent *evt) override;

    virtual void resizeEvent(QResizeEvent *evt) override;

private:

    void setViewModeImpl(ViewMode mode);

    void setDataAtMouse(QPoint coords);

    void calculateAxis();

    void calculateCellWidth();

    void calculateCellHeight();

    int availableWidth();

    int availableHeight();

    QPoint mouseToPlotCoordinates(QPoint mouse);

    void updateHover(QPoint mouse);

    GraphModel &mModel;
    ViewMode mMode;

    int mYAxisWidth;
    QRect mPlotRect;

    // data shown as bars
    bool mBarMode;
    bool mLines;

    int mMinValue;
    int mMaxValue;

    int mCellWidth;
    int mCellHeight;

    std::optional<QPoint> mMouseOver;
    QPoint mLastMouseCoords;

    QColor mLineColor;
    QColor mSampleColor;


};
