
#include "WaveEditor.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstddef>
#include <cmath>

#include <QFontDatabase>


WaveEditor::WaveEditor(WaveListModel &model, PianoInput &input, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mModel(model),
    mIgnoreNextUpdate(false),
    mLayout(QBoxLayout::TopToBottom),
    mCoordsLayout(QBoxLayout::LeftToRight),
    mCoordsLabel(),
    mWaveCombo(),
    mGraph(model),
    mWaveramGroup(tr("Wave RAM")),
    mWaveramLayout(QBoxLayout::LeftToRight),
    mWaveramEdit(),
    mClearButton(tr("Clear")),
    mPiano(input)
    
{

    // layout
    
    
    // row 0 - coordinate label + wave combobox
    mCoordsLayout.addWidget(&mCoordsLabel);
    mCoordsLayout.addWidget(&mWaveCombo);

    // row 2 - waveram edit
    mWaveramLayout.addWidget(&mWaveramEdit, 1);
    mWaveramLayout.addWidget(&mClearButton);
    mWaveramGroup.setLayout(&mWaveramLayout);

    mLayout.addLayout(&mCoordsLayout);
    mLayout.addWidget(&mGraph, 1);
    mLayout.addWidget(&mWaveramGroup);
    mLayout.addWidget(&mPiano);

    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(&mLayout);

    // settings
    setWindowTitle(tr("Wave Editor"));
    mWaveCombo.setModel(&mModel);
    // use system monospace font for the waveram line edit
    mWaveramEdit.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mWaveramEdit.setMaxLength(32);
    mWaveramEdit.setAlignment(Qt::AlignCenter);
    mWaveramEdit.setInputMask(QStringLiteral("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"));
    mWaveramEdit.setText(QStringLiteral("00000000000000000000000000000000"));

    // signals

    // coordinate label will get updated via WaveGraph signal
    connect(&mGraph, &WaveGraph::coordsTextChanged, &mCoordsLabel, &QLabel::setText);

    // update waveram text when the current waveform changes
    connect(&mModel, qOverload<QPoint>(&WaveListModel::waveformChanged), this, &WaveEditor::onSampleChanged);
    connect(&mModel, qOverload<>(&WaveListModel::waveformChanged), this, &WaveEditor::updateWaveramText);
    
    connect(&mWaveramEdit, &QLineEdit::textEdited, this, &WaveEditor::onWaveramEdited);
    
    connect(&mClearButton, &QPushButton::clicked, &mModel, &WaveListModel::clear);
    
    // mWaveSelect can also be used to change the current waveform
    connect(&mWaveCombo, qOverload<int>(&QComboBox::currentIndexChanged), &mModel, qOverload<int>(&WaveListModel::select));
    connect(&mModel, &WaveListModel::currentIndexChanged, this, &WaveEditor::selectionChanged);

    //setFixedSize(size());

    selectionChanged(model.currentIndex());
}

WaveEditor::~WaveEditor() {
}

PianoWidget& WaveEditor::piano() {
    return mPiano;
}

// when the user changes mWaveSelect or is set from the MainWindow
void WaveEditor::selectionChanged(int index) {
    if (index == -1) {
        close();
    } else {
        mWaveCombo.setCurrentIndex(index);
        updateWaveramText();
    }
}

void WaveEditor::onSampleChanged(QPoint point) {
    // user changed a sample using the graph control, so update the wave ram line edit
    auto text = mWaveramEdit.text();
    text.replace(point.x(), 1, QString::number(point.y(), 16).toUpper());
    mWaveramEdit.setText(text);
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
        mWaveramEdit.setText(QString::fromStdString(ss.str()));
        mGraph.update();
    } else {
        mIgnoreNextUpdate = false;
    }
}

//void WaveEditor::setFromPreset(Preset preset) {
//
//    // waveform to sample from
//    uint8_t presetData[32];
//    uint8_t waveData[32];
//
//    auto amplitude = mUi->mPresetAmpSpin->value();
//    switch (preset) {
//        case Preset::square:
//            {
//                uint8_t duty =  static_cast<uint8_t>(mUi->mPresetDutySpin->value() * (32.0 / 100.0));
//                std::fill_n(presetData, duty, amplitude);
//                std::fill_n(presetData + duty, 32 - duty, 0);
//            }
//            break;
//        case Preset::triangle:
//            {
//                float step = amplitude / 15.0f;
//                float sample = 0.0f;
//                // upwards
//                for (int i = 0; i != 16; ++i) {
//                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
//                    sample += step;
//                }
//                // and then back down
//                sample = static_cast<float>(amplitude);
//                for (int i = 16; i != 32; ++i) {
//                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
//                    sample -= step;
//                }
//            }
//            break;
//        case Preset::sawtooth:
//            {
//                float step = amplitude / 31.0f;
//                float sample = 0.0f;
//                for (int i = 0; i != 32; ++i) {
//                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
//                    sample += step;
//                }
//            }
//            break;
//        case Preset::sine:
//            {
//                // 1 period from 0 to 2 pi radians in 32 steps
//                float angle = 0.0f;
//                for (int i = 0; i != 32; ++i) {
//                    // translate sine from [-1, 1] to [0, 1] and apply amplitude
//                    presetData[i] = static_cast<uint8_t>(std::roundf(0.5f * amplitude * (sin(angle) + 1.0f)));
//                    angle += 0.1963495408f; // ==> 2pi / 32
//                }
//            }
//            break;
//
//    }
//
//    // get the phase and period settings
//    auto phase = mUi->mPresetPhaseSpin->value();
//    auto periods = mUi->mPresetPeriodSpin->value();
//
//    // start at the phase offset
//    int presetIndex = phase;
//    for (int i = 0; i != 32; ++i) {
//        // sample from the preset data
//        waveData[i] = presetData[presetIndex];
//        // set index to the next sample
//        presetIndex = (presetIndex + periods) & 0x1F;
//    }
//    mModel.setData(waveData);
//}

