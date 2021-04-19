
#pragma once

#include "core/model/OrderModel.hpp"
#include "core/PianoInput.hpp"
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/PatternGridHeader.hpp"


#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QScrollBar>
#include <QSpinBox>
#include <QToolBar>


class PatternEditor : public QFrame {

    Q_OBJECT

public:

    

    struct Actions {
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

    struct TrackerActions {
        QAction play;
        QAction restart;
        QAction playRow;
        QAction record;
    };

    explicit PatternEditor(PianoInput &input, OrderModel &model, QWidget *parent = nullptr);
    ~PatternEditor() = default;

    PatternGrid& grid();

    Actions& menuActions();

    TrackerActions& trackerActions();

    void setupMenu(QMenu &menu);

    void setColors(ColorTable const& colors);

signals:
    void octaveChanged(int octave);

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

    void keyReleaseEvent(QKeyEvent *evt) override;

    void wheelEvent(QWheelEvent *evt) override;

private slots:

    void hscrollAction(int action);
    void vscrollAction(int action);

private:
    Q_DISABLE_COPY(PatternEditor)

    PianoInput &mPianoIn;

    QVBoxLayout mLayout;
        QWidget mControls;
            QHBoxLayout mControlsLayout;
                QToolBar mToolbar;
                QLabel mOctaveLabel;
                QSpinBox mOctaveSpin;
                QLabel mEditStepLabel;
                QSpinBox mEditStepSpin;
                QCheckBox mLoopPatternCheck;
                QCheckBox mFollowModeCheck;
                QCheckBox mKeyRepeatCheck;
        QFrame mGridFrame;
            QGridLayout mGridLayout;
                PatternGridHeader mGridHeader;
                PatternGrid mGrid;
                QScrollBar mHScroll;
                QScrollBar mVScroll;

    QMenu mContextMenu;
    QMenu mTransposeMenu;
    Actions mActions;

    TrackerActions mTrackerActions;

    int mWheel;
    int mPageStep;

    int mPreviewKey;

};
