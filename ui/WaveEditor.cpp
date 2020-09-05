
#include <algorithm>
#include <sstream>
#include <cstddef>
#include <cmath>

#include <QFontDatabase>

#include "WaveEditor.hpp"


WaveEditor::WaveEditor(ModuleDocument *document, QWidget *parent) :
    mDocument(document),
    mCurrentWaveform(nullptr),
    mWavedata{0},
    QWidget(parent)
{
    setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    //WaveListModel *model = new WaveListModel(mod.waveTable(), this);
    mWaveSelect->setModel(document->waveListModel());

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
    connect(mWaveSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WaveEditor::selectionChanged);
    connect(mNameEdit, &QLineEdit::textEdited, this, &WaveEditor::nameEdited);

    // presets
    connect(mPresetSineButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sine); });
    connect(mPresetTriButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::triangle); });
    connect(mPresetSquareButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::square); });
    connect(mPresetSawButton, &QPushButton::clicked, this, [this] { setFromPreset(Preset::sawtooth); });
}

void WaveEditor::selectWaveform(const QModelIndex &index) {
    if (mWaveSelect->currentIndex() != index.row()) {
        mWaveSelect->setCurrentIndex(index.row());
        // get the waveform data
    }
}

// when the user changes mWaveSelect or is set from the MainWindow
void WaveEditor::selectionChanged(int index) {
    auto model = mDocument->waveListModel();
    
    mCurrentWaveform = model->waveform(index);
    auto data = mCurrentWaveform->data();
    // unpack the waveform into mWavedata
    for (int i = 0, j = 0; i != 16; ++i) {
        uint8_t samples = data[i];
        mWavedata[j++] = samples >> 4;
        mWavedata[j++] = samples & 0xF;
    }

    updateWaveramText();
    mNameEdit->setText(model->name(index));
}

void WaveEditor::nameEdited(const QString &text) {
    // update the name change to the model
    auto model = mDocument->waveListModel();
    model->setName(mWaveSelect->currentIndex(), text);
}

void WaveEditor::onSampleChanged(QPoint point) {
    // user changed a sample using the graph control, so update the wave ram line edit
    auto text = mWaveramEdit->text();
    text.replace(point.x(), 1, QString::number(point.y(), 16).toUpper());
    mWaveramEdit->setText(text);

    // update the current waveform's data
    if (mCurrentWaveform != nullptr) {
        int index = point.x() / 2;
        auto data = mCurrentWaveform->data();
        uint8_t sample = data[index];
        if (!!(point.x() & 1)) {
            sample = (sample & 0xF0) | point.y();
        } else {
            sample = (sample & 0x0F) | (point.y() << 4);
        }
        data[index] = sample;
    }
}

void WaveEditor::onWaveramEdited(const QString &text) {
    // user edited the wave ram line edit, update the wave sample data

    // probably a better way of doing this
    uint8_t pos = 0;
    for (auto ch : text) {
        
        QString s(ch);
        mWavedata[pos] = static_cast<uint8_t>(s.toUInt(nullptr, 16));
        ++pos;
    }
    // redraw the graph
    mWaveGraph->repaint();
    pack();
}


// function buttons

void WaveEditor::onClear() {
    std::fill_n(mWavedata.begin(), 32, 0);
    updateWaveramText();
    pack();
}

void WaveEditor::onInvert() {
    
    // invert is just the compliment
    for (auto &sample : mWavedata) {
        sample = (~sample) & 0xF;
    }
    updateWaveramText();
    pack();
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
    pack();
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
    pack();
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
                    presetData[i] = static_cast<uint8_t>(std::roundf(sample));
                    sample += step;
                }
                // and then back down
                sample = amplitude;
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
    pack();
}

void WaveEditor::pack() {
    if (mCurrentWaveform != nullptr) {
        auto data = mCurrentWaveform->data();
        for (int i = 0, j = 0; i != 16; ++i) {
            data[i] = (mWavedata[j++] << 4) | (mWavedata[j++]);
        }
        
    }
}
