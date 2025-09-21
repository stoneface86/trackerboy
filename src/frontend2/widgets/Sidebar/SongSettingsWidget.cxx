
#include "widgets/Sidebar/SongSettingsWidget.hxx"
#include "utils/connectutils.hxx"
#include "utils/string.hxx"

#include <QGridLayout>
#include <QVBoxLayout>

SongSettingsWidget::SongSettingsWidget(SongModel *model, QWidget *parent)
    : QWidget(parent)
    , _model(model) {

    auto makeSpinBox = [](int min, int max) -> QSpinBox * {
        auto result = new QSpinBox;
        result->setRange(min, max);
        return result;
    };

    _nameEdit = new QLineEdit;
    _rpbSpin = makeSpinBox(1, 255);
    _rpmSpin = makeSpinBox(1, 255);
    _patternSizeSpin = makeSpinBox(1, 256);
    _speedSpin = new CustomSpinBox;
    _speedSpin->setDigits(2);
    _speedSpin->setDisplayIntegerBase(16);
    _speedSpin->setRange(B::SpeedLow, B::SpeedHigh);
    _speedSpin->setPrefix("$");

    _speedLabel = new QLabel;
    _speedLabel->setAlignment(Qt::AlignHCenter);
    _tempoLabel = new QLabel;
    _tempoLabel->setAlignment(Qt::AlignHCenter);

    _tickrateOverride = new TickrateForm;
    _tickrateOverride->setCheckable(true);

    auto layout = new QVBoxLayout;

    auto nameGroup = new QGroupBox(tr("Name"));
    auto nameLayout = new QVBoxLayout;
    nameLayout->addWidget(_nameEdit);
    nameGroup->setLayout(nameLayout);

    auto patternGroup = new QGroupBox(tr("Pattern"));
    auto patternLayout = new QGridLayout;
    auto addSettingRow = [](QGridLayout *layout, QString const &text,
                            QWidget *widget, int row) {
        auto label = new QLabel(text);
        label->setBuddy(widget);
        layout->addWidget(label, row, 0);
        layout->addWidget(widget, row, 1);
    };
    addSettingRow(patternLayout, tr("Row/beat"), _rpbSpin, 0);
    addSettingRow(patternLayout, tr("Row/measure"), _rpmSpin, 1);
    addSettingRow(patternLayout, tr("Rows"), _patternSizeSpin, 2);
    addSettingRow(patternLayout, tr("Speed"), _speedSpin, 3);
    patternLayout->addWidget(_speedLabel, 4, 0);
    patternLayout->addWidget(_tempoLabel, 4, 1);
    patternGroup->setLayout(patternLayout);
    layout->addWidget(nameGroup);
    layout->addWidget(patternGroup);
    layout->addWidget(_tickrateOverride);
    layout->addStretch();

    setLayout(layout);

    auto const doc = _model->document();
    lazyconnect(doc, songChanged, this, onSongChange);
    lazyconnect(_nameEdit, textEdited, _model, setName);
    lazyconnect(_model, speedChanged, this, setSpeedDisplay);
    lazyconnect(_model, speedChanged, this, setTempoDisplay);
    lazyconnect(_model, rowsPerBeatChanged, this, setTempoDisplay);
    lazyconnect(_model, tickrateChanged, this, setTempoDisplay);

    lazyconnect(_rpbSpin, valueChanged, _model, setRowsPerBeat);
    lazyconnect(_rpmSpin, valueChanged, _model, setRowsPerMeasure);
    lazyconnect(_speedSpin, valueChanged, _model, setSpeed);
    lazyconnect(_patternSizeSpin, valueChanged, _model, setPatternSize);
    lazyconnect(_tickrateOverride, toggled, this, tickrateToggled);
    lazyconnect(_tickrateOverride, tickrateChanged, this, updateTickrate);

    //  initialize form data
    onSongChange();
}

void SongSettingsWidget::loadName() {
    _nameEdit->setText(_model->name());
}

void SongSettingsWidget::onSongChange() {
    loadName();
    _rpbSpin->setValue(_model->rowsPerBeat());
    _rpmSpin->setValue(_model->rowsPerMeasure());
    _patternSizeSpin->setValue(_model->patternSize());
    _speedSpin->setValue(_model->speed());
    _tickrateOverride->setChecked(_model->hasTickrate());
    _tickrateOverride->setTickrate(_model->tickrate());
    setSpeedDisplay();
    setTempoDisplay();
}

void SongSettingsWidget::setSpeedDisplay() {
    _speedLabel->setText(speedToString(_model->speedFloat()));
}

void SongSettingsWidget::setTempoDisplay() {
    _tempoLabel->setText(tempoToString(_model->tempo()));
}

void SongSettingsWidget::tickrateToggled(bool on) {
    if (on) {
        _model->setTickrate(_tickrateOverride->tickrate());
    } else {
        _model->clearTickrate();
    }
}

void SongSettingsWidget::updateTickrate() {
    _model->setTickrate(_tickrateOverride->tickrate());
}