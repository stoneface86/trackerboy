
#pragma once

#include "model/SongModel.hxx"
#include "widgets/CustomSpinBox.hxx"
#include "widgets/TickrateForm.hxx"

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

class SongSettingsWidget : public QWidget {

public:
    explicit SongSettingsWidget(SongModel *model, QWidget *parent = nullptr);

private:
    void onSongChange();
    void loadName();
    void setSpeedDisplay();
    void setTempoDisplay();
    void tickrateToggled(bool on);
    void updateTickrate();

    Q_DISABLE_COPY(SongSettingsWidget)
    SongModel *_model;

    QLineEdit *_nameEdit;
    QSpinBox *_rpbSpin;
    QSpinBox *_rpmSpin;
    QSpinBox *_patternSizeSpin;
    CustomSpinBox *_speedSpin;
    QLabel *_speedLabel;
    QLabel *_tempoLabel;

    TickrateForm *_tickrateOverride;
};
