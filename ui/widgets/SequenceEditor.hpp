
#pragma once

#include <QGridLayout>
#include <QScrollBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class SequenceEditor : public QWidget {

    Q_OBJECT

public:

    enum ViewMode {
        SampleView, // waveforms, arpeggio
        BarView     // duty, pitch, panning
    };

    explicit SequenceEditor(QWidget *parent = nullptr);

//     void setRange(int min, int max);

//     void setScrollY(int y);

//     void setScrollX(int x);

//     void setViewMode(ViewMode mode);

// public slots:

//     void setSize(int size);

//     void setData(QString const& data);

private:

    QGridLayout mLayout;
        // row 0
        // loop select widget

        // row 1
        QVBoxLayout mAxisLayout;
            QLabel mMaxLabel;
            QLabel mMinLabel;
        //SequenceGraph mGraph;
        QScrollBar mVScroll;

        // row 2
        QScrollBar mHScroll;


    std::vector<uint8_t> mData;

    int mCellWidth;
    int mCellHeight;
};


