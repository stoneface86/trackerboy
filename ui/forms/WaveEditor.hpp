
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "core/model/WaveListModel.hpp"
#include "widgets/PianoWidget.hpp"
#include "widgets/WaveGraph.hpp"

#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <array>

class WaveEditor : public QDialog {

    Q_OBJECT

public:
    explicit WaveEditor(WaveListModel &model, PianoInput &input, QWidget *parent = nullptr);
    ~WaveEditor();

    PianoWidget& piano();

private slots:
    void onSampleChanged(QPoint sample);
    void onWaveramEdited(const QString &text);
    void selectionChanged(int index);

private:

    enum class Preset {
        square,
        triangle,
        sawtooth,
        sine
    };

    void updateWaveramText();

    //void setFromPreset(Preset preset);

    WaveListModel &mModel;
    bool mIgnoreNextUpdate;

    // widgets
    // indentation is used to show parent/child relationships

    QBoxLayout mLayout;
        QBoxLayout mCoordsLayout;
            QLabel mCoordsLabel;
            QComboBox mWaveCombo;
        WaveGraph mGraph;
        QGroupBox mWaveramGroup;
            QBoxLayout mWaveramLayout;
                QLineEdit mWaveramEdit;
                QPushButton mClearButton;
        PianoWidget mPiano;
};
