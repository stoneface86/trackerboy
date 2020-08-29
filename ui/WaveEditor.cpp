
#include <algorithm>
#include <sstream>
#include <cstddef>
#include <cmath>

#include <QFontDatabase>

#include "WaveEditor.hpp"


WaveEditor::WaveEditor() :
    mWavedata{0},
    QWidget()
{
    setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    // let the graph widget use our sample data to display
    mWaveGraph->setData(mWavedata.data());

    // use system monospace font for the waveram line edit
    mWaveramEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(mWaveGraph, &WaveGraph::coordsTextChanged, mCoordsLabel, &QLabel::setText);
    connect(mWaveGraph, &WaveGraph::sampleChanged, this, &WaveEditor::onSampleChanged);
    connect(mWaveramEdit, &QLineEdit::textEdited, this, &WaveEditor::onWaveramEdited);
    connect(mClearButton, &QPushButton::clicked, this, &WaveEditor::onClear);
    connect(mInvertButton, &QPushButton::clicked, this, &WaveEditor::onInvert);
    connect(mRotateLeftButton, &QPushButton::clicked, this, &WaveEditor::onRotateLeft);
    connect(mRotateRightButton, &QPushButton::clicked, this, &WaveEditor::onRotateRight);

    // presets
    connect(mPresetSineButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sine); });
    connect(mPresetTriButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::triangle); });
    connect(mPresetSquareButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::square); });
    connect(mPresetSawButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sawtooth); });
}

void WaveEditor::onSampleChanged(QPoint point) {
    // user changed a sample using the graph control, so update the wave ram line edit
    auto text = mWaveramEdit->text();
    text.replace(point.x(), 1, QString::number(point.y(), 16).toUpper());
    mWaveramEdit->setText(text);
}

void WaveEditor::onWaveramEdited(const QString &text) {
    // user edited the wave ram line edit, update the wave sample data

    // probably a better way of doing this
    uint8_t pos = 0;
    for (auto ch : text) {
        
        QString s(ch);
        mWavedata[pos] = s.toUInt(nullptr, 16);
        ++pos;
    }
    // redraw the graph
    mWaveGraph->repaint();
}


// function buttons

void WaveEditor::onClear() {
    std::fill_n(mWavedata.begin(), 32, 0);
    updateWaveramText();
}

void WaveEditor::onInvert() {
    
    // invert is just the compliment
    for (auto &sample : mWavedata) {
        sample = (~sample) & 0xF;
    }
    updateWaveramText();
}

void WaveEditor::onRotateLeft() {
    // save the first sample for later
    uint8_t sampleFirst = mWavedata[0];
    for (size_t i = 0; i != 31; ++i) {
        // current sample gets the one ahead of it
        mWavedata[i] = mWavedata[i + 1];
    }
    // wrap-around
    mWavedata[31] = sampleFirst;
    updateWaveramText();
}

void WaveEditor::onRotateRight() {
    // save the last sample for later
    uint8_t sampleEnd = mWavedata[31];
    for (size_t i = 31; i != 0; --i) {
        // current sample gets the one before it
        mWavedata[i] = mWavedata[i - 1];
    }
    // wrap-around
    mWavedata[0] = sampleEnd;
    updateWaveramText();
}

void WaveEditor::updateWaveramText() {
    // probably should use QString but eh
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (auto sample : mWavedata) {
        ss << static_cast<int>(sample);
    }
    mWaveramEdit->setText(QString::fromStdString(ss.str()));
    mWaveGraph->repaint();
}

void WaveEditor::setFromPreset(Preset preset) {

    // waveform to sample from
    uint8_t presetData[32];

    auto amplitude = mPresetAmpSpin->value();
    switch (preset) {
        case Preset::square:
            {
                uint8_t duty =  static_cast<uint8_t>(mPresetDutySpin->value() * (32.0 / 100.0));
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
                    presetData[i] = std::roundf(sample);
                    sample += step;
                }
                // and then back down
                for (int i = 16; i != 32; ++i) {
                    presetData[i] = std::roundf(sample);
                    sample -= step;
                }
            }
            break;
        case Preset::sawtooth:
            {
                float step = amplitude / 31.0f;
                float sample = 0.0f;
                for (int i = 0; i != 32; ++i) {
                    presetData[i] = std::roundf(sample);
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
                    presetData[i] = std::roundf(0.5f * amplitude * (sin(angle) + 1.0f));
                    angle += 0.1963495408f; // ==> 2pi / 32
                }
            }
            break;

    }

    // get the phase and period settings
    auto phase = mPresetPhaseSpin->value();
    auto periods = mPresetPeriodSpin->value();

    // start at the phase offset
    int presetIndex = phase;
    for (int i = 0; i != 32; ++i) {
        // sample from the preset data
        mWavedata[i] = presetData[presetIndex];
        // set index to the next sample
        presetIndex = (presetIndex + periods) & 0x1F;
    }

    updateWaveramText();
}
