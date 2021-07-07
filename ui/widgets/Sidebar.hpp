
#pragma once

class ModuleDocument;

#include "widgets/visualizers/AudioScope.hpp"
#include "widgets/CustomSpinBox.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

//
// Composite widget for the tracker sidebar. This sits beside the pattern editor
// and contains the order editor, song settings, and visualizers.
//
class Sidebar : public QWidget {

    Q_OBJECT

public:

    explicit Sidebar(QWidget *parent = nullptr);

    void setDocument(ModuleDocument *doc);

    AudioScope& scope();


signals:
    void orderMenuRequested(QPoint const& pos);

    void patternJumpRequested(int pattern);

private:

    void currentIndexChanged(QModelIndex const& index);
    void currentChanged(QModelIndex const &current, QModelIndex const &prev);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void setTempoLabel(float tempo);
    float calcActualTempo(float speed);

    void updatePatternsSpin();


    ModuleDocument *mDocument;
    bool mIgnoreSelect;
    bool mSpeedLock;
    
    QVBoxLayout mLayout;
        AudioScope mScope;
        QGroupBox mOrderGroup;
            QVBoxLayout mOrderLayout;
                QHBoxLayout mOrderButtonLayout;
                    QToolBar mOrderToolbar;
                        QAction mActionIncrement;
                        QAction mActionDecrement;
                    CustomSpinBox mSetSpin;
                    QPushButton mSetButton;
                QTableView mOrderView;
        QGroupBox mSongGroup;
            QGridLayout mSongLayout;
                QLabel mRowsPerBeatLabel;
                QSpinBox mRowsPerBeatSpin;
                QLabel mRowsPerMeasureLabel;
                QSpinBox mRowsPerMeasureSpin;
                QLabel mSpeedLabel;
                CustomSpinBox mSpeedSpin;
                QLabel mSpeedActual;
                QLabel mTempoLabel;
                QSpinBox mTempoSpin;
                QLabel mTempoActual;
                QLabel mRowsLabel;
                QSpinBox mRowsSpin;
                QLabel mPatternsLabel;
                QSpinBox mPatternsSpin;

        //AudioScope mVisualizer;

        //AudioScope mLeftScope;
        //AudioScope mRightScope;

    


};
