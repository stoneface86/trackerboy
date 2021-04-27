
#include "widgets/module/ModuleSettingsWidget.hpp"

ModuleSettingsWidget::ModuleSettingsWidget(ModuleDocument &doc, QWidget *parent) :
    QWidget(parent),
    mDocument(doc),
    mLayout(),
    mInfoGroup(tr("Information")),
    mInfoLayout(),
    mTitleEdit(),
    mArtistEdit(),
    mCopyrightEdit(),
    mSongGroup(tr("Song settings")),
    mSongLayout(),
    mRowsPerBeatSpin(),
    mRowsPerMeasureSpin(),
    mSpeedLayout(),
    mSpeedSpin(),
    mTempoActualEdit(),
    mTempoSpin(),
    mTempoCalcButton(tr("Calculate speed")),
    mPatternSpin(),
    mRowsPerPatternSpin(),
    mEngineGroup(tr("Engine settings")),
    mEngineLayout(),
    mFramerateChoiceLayout(),
    mFramerateDmgRadio(tr("DMG (59.7 Hz)")),
    mFramerateSgbRadio(tr("SGB (61.1 Hz)")),
    mFramerateCustomLayout(),
    mFramerateCustomRadio(tr("Custom")),
    mFramerateCustomSpin(),
    mCommentsGroup(tr("Comments")),
    mCommentsLayout(),
    mCommentsEdit(),
    mFramerateButtons()
{
    // layout

    mInfoLayout.addRow(tr("Title:"), &mTitleEdit);
    mInfoLayout.addRow(tr("Artist:"), &mArtistEdit);
    mInfoLayout.addRow(tr("Copyright:"), &mCopyrightEdit);
    mInfoGroup.setLayout(&mInfoLayout);


    mSongLayout.addRow(tr("Rows per beat"), &mRowsPerBeatSpin);
    mSongLayout.addRow(tr("Rows per measure"), &mRowsPerMeasureSpin);

    mSpeedLayout.addWidget(&mSpeedSpin, 0, 0);
    mSpeedLayout.addWidget(&mTempoActualEdit, 0, 1);
    mSpeedLayout.addWidget(&mTempoSpin, 1, 0);
    mSpeedLayout.addWidget(&mTempoCalcButton, 1, 1);
    mSpeedLayout.setColumnStretch(0, 1);
    mSpeedLayout.setColumnStretch(1, 1);
    mSongLayout.addRow(tr("Speed"), &mSpeedLayout);

    mSongLayout.addRow(tr("Patterns"), &mPatternSpin);
    mSongLayout.addRow(tr("Rows"), &mRowsPerPatternSpin);
    mSongGroup.setLayout(&mSongLayout);


    mFramerateCustomLayout.addWidget(&mFramerateCustomRadio);
    mFramerateCustomLayout.addWidget(&mFramerateCustomSpin);
    mFramerateCustomLayout.addStretch();

    mFramerateChoiceLayout.addWidget(&mFramerateDmgRadio);
    mFramerateChoiceLayout.addWidget(&mFramerateSgbRadio);
    mFramerateChoiceLayout.addLayout(&mFramerateCustomLayout);

    mEngineLayout.addRow(tr("Framerate"), &mFramerateChoiceLayout);
    mEngineGroup.setLayout(&mEngineLayout);

    mCommentsLayout.addWidget(&mCommentsEdit, 0, 0);
    mCommentsGroup.setLayout(&mCommentsLayout);

    mLayout.addWidget(&mInfoGroup, 0, 0);
    mLayout.addWidget(&mSongGroup, 1, 0);
    mLayout.addWidget(&mEngineGroup, 2, 0);
    mLayout.addWidget(&mCommentsGroup, 0, 1, 3, 1);
    
    setLayout(&mLayout);

    mFramerateButtons.addButton(&mFramerateDmgRadio, static_cast<int>(trackerboy::System::dmg));
    mFramerateButtons.addButton(&mFramerateSgbRadio, static_cast<int>(trackerboy::System::sgb));
    mFramerateButtons.addButton(&mFramerateCustomRadio, static_cast<int>(trackerboy::System::custom));

    // settings

    mTitleEdit.setMaxLength(32);
    mArtistEdit.setMaxLength(32);
    mCopyrightEdit.setMaxLength(32);

    mRowsPerBeatSpin.setRange(1, 255);
    mRowsPerMeasureSpin.setRange(1, 255);
    mSpeedSpin.setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin.setDisplayIntegerBase(16);
    mSpeedSpin.setPrefix("0x");
    mTempoSpin.setRange(1, 10000);
    mTempoSpin.setValue(150);
    mTempoSpin.setSuffix(" BPM");
    mTempoActualEdit.setReadOnly(true);
    mPatternSpin.setRange(1, trackerboy::MAX_PATTERNS);
    mRowsPerPatternSpin.setRange(1, 256);

    mFramerateCustomSpin.setValue(30);
    mFramerateCustomSpin.setRange(1, 1024);
    mFramerateCustomSpin.setSuffix(tr(" Hz"));

    // now load settings from the module
    auto const& mod = mDocument.mod();
    //mTitleEdit.setText(QString::fromStdString(mod.title()));

    auto &song = mod.song();
    mRowsPerBeatSpin.setValue(song.rowsPerBeat());
    mRowsPerMeasureSpin.setValue(song.rowsPerMeasure());
    mSpeedSpin.setValue(song.speed());
    mPatternSpin.setValue((int)song.order().size());
    mRowsPerPatternSpin.setValue(song.patterns().rowSize());

    auto sys = mod.system();
    switch (sys) {
        case trackerboy::System::dmg:
            mFramerateDmgRadio.setChecked(true);
            break;
        case trackerboy::System::sgb:
            mFramerateSgbRadio.setChecked(true);
            break;
        case trackerboy::System::custom:
            mFramerateCustomRadio.setChecked(true);
            mFramerateCustomSpin.setValue(mod.customFramerate());
            break;
    }
    mFramerateCustomSpin.setEnabled(sys == trackerboy::System::custom);

    calculateActualTempo();

    // connections

    connect(&mTempoCalcButton, &QPushButton::clicked, this, &ModuleSettingsWidget::calculateTempo);
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(&mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ModuleSettingsWidget::calculateActualTempo);
    connect(&mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ModuleSettingsWidget::calculateActualTempo);
    connect(&mFramerateButtons, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &ModuleSettingsWidget::framerateButtonToggled);
    connect(&mFramerateCustomSpin, qOverload<int>(&QSpinBox::valueChanged), this, &ModuleSettingsWidget::customFramerateSpinChanged);
}

void ModuleSettingsWidget::calculateTempo() {
    // speed = (framerate * 60) / (tempo * rpb)

    float speed = (mDocument.mod().framerate() * 60.0f) / (mTempoSpin.value() * mRowsPerBeatSpin.value());
    // convert to fixed point
    int speedFixed = std::clamp(static_cast<int>(roundf(speed * 16.0f)), (int)trackerboy::SPEED_MIN, (int)trackerboy::SPEED_MAX);
    mSpeedSpin.setValue(speedFixed);

}

void ModuleSettingsWidget::calculateActualTempo() {
    // tempo = (framerate * 60) / (speed * rpb)
    // convert fixed point to floating point
    float speed = trackerboy::speedToFloat((trackerboy::Speed)mSpeedSpin.value());
    float tempo = (mDocument.mod().framerate() * 60.0f) / (speed * mRowsPerBeatSpin.value());
    mTempoActualEdit.setText(QString("%1 BPM").arg(tempo, 0, 'f', 2));
}

void ModuleSettingsWidget::framerateButtonToggled(QAbstractButton *button, bool checked) {
    if (checked) {
        auto sys = static_cast<trackerboy::System>(mFramerateButtons.id(button));

        bool const isCustom = sys == trackerboy::System::custom;
        mFramerateCustomSpin.setEnabled(isCustom);

        auto ctx = mDocument.beginEdit();
        auto& mod = mDocument.mod();
        if (isCustom) {
            mod.setFramerate((uint16_t)mFramerateCustomSpin.value());
        } else {
            mod.setFramerate(sys);
        }

        calculateActualTempo();
    }
}

void ModuleSettingsWidget::customFramerateSpinChanged(int value) {
    auto ctx = mDocument.beginEdit();
    mDocument.mod().setFramerate((uint16_t)value);
    calculateActualTempo();
}
