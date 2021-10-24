#pragma once

#include "core/model/graph/GraphModel.hpp"
#include "core/Palette.hpp"

#include <QAbstractScrollArea>
#include <QPoint>

#include <optional>

class GraphEdit : public QAbstractScrollArea {

    Q_OBJECT

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

    void setColors(Palette const& pal);

    void setViewMode(ViewMode mode);

    int minimumValue() const;

    int maximumValue() const;


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

    // highlight interval for rows in sample mode
    int mAlternateInterval;

    std::optional<QPoint> mMouseOver;
    QPoint mLastMouseCoords;

    QColor mBackgroundColor;
    QColor mAlternateColor;
    QColor mLineColor;
    QColor mSampleColor;


};
