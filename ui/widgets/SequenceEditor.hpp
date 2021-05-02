
#pragma once

#include <QWidget>

class SequenceEditor : public QWidget {

    Q_OBJECT

public:

    enum ViewMode {
        SampleView,
        BarView
    };

    explicit SequenceEditor(QWidget *parent = nullptr);

    void setRange(int min, int max);

    void setViewMode(ViewMode mode);

public slots:

    void setSize(int size);

    void setData(QString const& data);

protected:

    virtual void paintEvent(QPaintEvent *evt) override;

private:
    std::vector<uint8_t> mData;

    bool mDragging;

    uint8_t mCurX;
    uint8_t mCurY;

    QRect mPlotRect;

    int mCellWidth;
    int mCellHeight;
};


