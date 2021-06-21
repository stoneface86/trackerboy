
#pragma once

class ModuleDocument;

#include "widgets/CustomSpinBox.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QTableView>
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


signals:
    void orderMenuRequested(QPoint const& pos);

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
        QGroupBox mOrderGroup;
            QVBoxLayout mOrderLayout;
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
