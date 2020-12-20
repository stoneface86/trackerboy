
#pragma once

#include "model/OrderModel.hpp"
#include "widgets/grid/PatternGrid.hpp"
#include "Config.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QMenu>
#include <QScrollBar>


class PatternEditor : public QFrame {

    Q_OBJECT

public:

    struct Actions {

        QAction undo;
        QAction redo;
        QAction cut;
        QAction copy;
        QAction paste;
        QAction pasteMix;
        QAction delete_;
        QAction selectAll;
        QAction noteIncrease;
        QAction noteDecrease;
        QAction octaveIncrease;
        QAction octaveDecrease;
        QAction reverse;

    };

    explicit PatternEditor(SongListModel &model, ColorTable const &colortable, QWidget *parent = nullptr);
    ~PatternEditor() = default;

    Actions& menuActions();

    void setAppearance(Config::Appearance const& appearance);

    void setupMenu(QMenu &menu);

//public slots:

    //void onCut();

    //void onCopy();

    //void onPaste();

    //void onPasteMix();

    //void onDelete();

    //void onSelectAll();

    //void onIncreaseNote();

    //void onDecreaseNote();

    //void onIncreaseOctave();

    //void onDecreaseOctave();

    //void onReverse();

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

    QMenu mContextMenu;
    QMenu mTransposeMenu;
    Actions mActions;

    int mWheel;
    int mPageStep;

};
