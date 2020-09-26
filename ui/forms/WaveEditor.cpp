
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstddef>
#include <cmath>

#include <QFontDatabase>

#include "WaveEditor.hpp"

#pragma warning(push, 0)
#include "designer/ui_WaveEditor.h"
#pragma warning(pop)


WaveEditor::WaveEditor(WaveListModel &model, QWidget *parent) :
    mUi(new Ui::WaveEditor()),
    mModel(model),
    mIgnoreNextUpdate(false),
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    mUi->setupUi(this);

    //WaveListModel *model = new WaveListModel(mod.waveTable(), this);
    mUi->mWaveSelect->setModel(&mModel);
    mUi->mWaveGraph->setModel(&mModel);

    // use system monospace font for the waveram line edit
    mUi->mWaveramEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    // coordinate label will get updated via WaveGraph signal
    connect(mUi->mWaveGraph, &WaveGraph::coordsTextChanged, mUi->mCoordsLabel, &QLabel::setText);

    // update waveram text when the current waveform changes
    connect(&mModel, QOverload<QPoint>::of(&WaveListModel::waveformChanged), this, &WaveEditor::onSampleChanged);
    connect(&mModel, QOverload<>::of(&WaveListModel::waveformChanged), this, &WaveEditor::updateWaveramText);
    
    connect(mUi->mWaveramEdit, &QLineEdit::textEdited, this, &WaveEditor::onWaveramEdited);
    // connect buttons to model functions
    connect(mUi->mClearButton, &QPushButton::clicked, &mModel, &WaveListModel::clear);
    connect(mUi->mInvertButton, &QPushButton::clicked, &mModel, &WaveListModel::invert);
    connect(mUi->mRotateLeftButton, &QPushButton::clicked, &mModel, &WaveListModel::rotateLeft);
    connect(mUi->mRotateRightButton, &QPushButton::clicked, &mModel, &WaveListModel::rotateRight);
    // mWaveSelect can also be used to change the current waveform
    connect(mUi->mWaveSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), &mModel, QOverload<int>::of(&WaveListModel::select));
    connect(&mModel, &WaveListModel::currentIndexChanged, this, &WaveEditor::selectionChanged);
    connect(mUi->mNameEdit, &QLineEdit::textEdited, &mModel, &WaveListModel::rename);

    // presets
    connect(mUi->mPresetSineButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sine); });
    connect(mUi->mPresetTriButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::triangle); });
    connect(mUi->mPresetSquareButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::square); });
    connect(mUi->mPresetSawButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sawtooth); });

    setFixedSize(size());
}

WaveEditor::~WaveEditor() {
    delete mUi;
}

PianoWidget* WaveEditor::piano() {
    return mUi->mPiano;
}

// when the user changes mWaveSelect or is set from the MainWindow
void WaveEditor::selectionChanged(int index) {
    if (index != -1) {
        mUi->mWaveSelect->setCurrentIndex(index);
        updateWaveramText();
        mUi->mNameEdit->setText(mModel.name());
    }
}

void WaveEditor::onSampleChanged(QPoint point) {
    // user changed a sample using the graph control, so update the wave ram line edit
    auto text = mUi->mWaveramEdit->text();
    text.replace(point.x(), 1, QString::number(point.y(), 16).toUpper());
    mUi->mWaveramEdit->setText(text);
}

void WaveEditor::onWaveramEdited(const QString &text) {
    mIgnoreNextUpdate = true;
    mModel.setData(text);
}


void WaveEditor::updateWaveramText() {
    // probably should use QString but eh
    if (!mIgnoreNextUpdate) {
        auto wavedata = mModel.currentWaveform()->data();
        std::stringstream ss;
        ss << std::hex << std::uppercase;
        for (int i = 0; i != trackerboy::Gbs::WAVE_RAMSIZE; ++i) {
            ss << std::setw(2) << std::setfill('0') << static_cast<unsigned>(wavedata[i]);
        }
        mUi->mWaveramEdit->setText(QString::fromStdString(ss.str()));
        mUi->mWaveGraph->repaint();
    } else {
        mIgnoreNextUpdate = false;
    }
}

void WaveEditor::setFromPreset(Preset preset) {

    // waveform to sample from
    uint8_t presetData[32];
    uint8_t waveData[32];

    auto amplitude = mUi->mPresetAmpSpin->value();
    switch (preset) {
        case Preset::square:
            {
                uint8_t duty =  static_cast<uint8_t>(mUi->mPresetDutySpin->value() * (32.0 / 100.0));
                std::fill_n(presetData, duty, amplitude);
                std::fill_n(presetData + duty, 32 - duty, 0);
            }
            break;
        case Preset::triangle:
            {
                float step = amplitude / 15.0f;
                float sample = 0.0f;
                // upwards
                for (int i = 0; i != 16; ++i) {
                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
                    sample += step;
                }
                // and then back down
                sample = static_cast<float>(amplitude);
                for (int i = 16; i != 32; ++i) {
                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
                    sample -= step;
                }
            }
            break;
        case Preset::sawtooth:
            {
                float step = amplitude / 31.0f;
                float sample = 0.0f;
                for (int i = 0; i != 32; ++i) {
                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
                    sample += step;
                }
            }
            break;
        case Preset::sine:
            {
                // 1 period from 0 to 2 pi radians in 32 steps
                float angle = 0.0f;
                for (int i = 0; i != 32; ++i) {
                    // translate sine from [-1, 1] to [0, 1] and apply amplitude
                    presetData[i] = static_cast<uint8_t>(std::roundf(0.5f * amplitude * (sin(angle) + 1.0f)));
                    angle += 0.1963495408f; // ==> 2pi / 32
                }
            }
            break;

    }

    // get the phase and period settings
    auto phase = mUi->mPresetPhaseSpin->value();
    auto periods = mUi->mPresetPeriodSpin->value();

    // start at the phase offset
    int presetIndex = phase;
    for (int i = 0; i != 32; ++i) {
        // sample from the preset data
        waveData[i] = presetData[presetIndex];
        // set index to the next sample
        presetIndex = (presetIndex + periods) & 0x1F;
    }
    mModel.setData(waveData);
}

