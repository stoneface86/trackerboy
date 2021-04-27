
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QButtonGroup>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>


class ModuleSettingsWidget : public QWidget {

    Q_OBJECT

public:

    explicit ModuleSettingsWidget(ModuleDocument &doc, QWidget *parent = nullptr);

private slots:

    void calculateTempo();

    void calculateActualTempo();

    void framerateButtonToggled(QAbstractButton *button, bool checked);

    void customFramerateSpinChanged(int value);

private:

    ModuleDocument &mDocument;

    QGridLayout mLayout;
        QGroupBox mInfoGroup;
            QFormLayout mInfoLayout;
                QLineEdit mTitleEdit;
                QLineEdit mArtistEdit;
                QLineEdit mCopyrightEdit;
        QGroupBox mSongGroup;
            QFormLayout mSongLayout;
                QSpinBox mRowsPerBeatSpin;
                QSpinBox mRowsPerMeasureSpin;
                QGridLayout mSpeedLayout;
                    QSpinBox mSpeedSpin;            // 0, 0
                    QLineEdit mTempoActualEdit;     // 0, 1
                    QSpinBox mTempoSpin;            // 1, 0
                    QPushButton mTempoCalcButton;   // 1, 1    
                QSpinBox mPatternSpin;
                QSpinBox mRowsPerPatternSpin;
        QGroupBox mEngineGroup;
            QFormLayout mEngineLayout;
                QVBoxLayout mFramerateChoiceLayout;
                    QRadioButton mFramerateDmgRadio;
                    QRadioButton mFramerateSgbRadio;
                    QHBoxLayout mFramerateCustomLayout;
                        QRadioButton mFramerateCustomRadio;
                        QSpinBox mFramerateCustomSpin;
        QGroupBox mCommentsGroup;
            QGridLayout mCommentsLayout;
                QPlainTextEdit mCommentsEdit;


    QButtonGroup mFramerateButtons;
};
