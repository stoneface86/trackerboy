
#pragma once

#include "model/OrderModel.hpp"

#include "widgets/grid/PatternGrid.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QScrollBar>


class PatternEditor : public QFrame {

    Q_OBJECT

public:
    explicit PatternEditor(SongListModel &model, QWidget *parent = nullptr);
    ~PatternEditor() = default;

protected:

    void keyPressEvent(QKeyEvent *evt) override;

    void wheelEvent(QWheelEvent *evt) override;

private slots:

    void hscrollAction(int action);
    void vscrollAction(int action);

private:

    QGridLayout mLayout;
        PatternGrid mGrid;
        QScrollBar mHScroll;
        QScrollBar mVScroll;

    int mWheel;
    int mPageStep;

};
