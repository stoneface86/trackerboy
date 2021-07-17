
#pragma once

#include "core/model/OrderModel.hpp"
#include "core/model/SongModel.hpp"

#include "widgets/visualizers/AudioScope.hpp"
#include "widgets/CustomSpinBox.hpp"
#include "widgets/OrderEditor.hpp"

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

    AudioScope& scope();

    OrderEditor& orderEditor();

private:

    void currentIndexChanged(QModelIndex const& index);
    void currentChanged(QModelIndex const &current, QModelIndex const &prev);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void setTempoLabel(float tempo);
    float calcActualTempo(float speed);

    void updatePatternsSpin();

    bool mIgnoreSelect;
    bool mSpeedLock;
    
    QVBoxLayout mLayout;
        AudioScope mScope;
        QGroupBox mOrderGroup;
            QVBoxLayout mOrderLayout;
                OrderEditor mOrderEditor;
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
