
#include "widgets/editors/WaveEditor.hpp"

#include <QtDebug>
#include <QFontDatabase>

constexpr int PRESET_PULSE50 = 0;
constexpr int PRESET_PULSE25 = 1;
constexpr int PRESET_TRIANGLE = 2;
constexpr int PRESET_SAWTOOTH = 3;
constexpr int PRESET_SINE = 4;

static trackerboy::Waveform::Data const PRESETS[] = {
    // 50% duty
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 25% duty
    {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // triangle
    {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
    // sawtooth
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
    // sine
    {0x89, 0xAC, 0xDE, 0xFF, 0xFF, 0xED, 0xCB, 0xA8, 0x75, 0x43, 0x21, 0x00, 0x00, 0x12, 0x35, 0x67}
    
};

WaveEditor::WaveEditor(PianoInput const& input, QWidget *parent) :
    BaseEditor(input, tr("waveform"), parent),
    mWaveModel(),
    mLayout(),
    mGraph(mWaveModel),
    mPresetLayout(),
    mSquare50Button(tr("50% Duty")),
    mSquare25Button(tr("25% Duty")),
    mTriangleButton(tr("Triangle")),
    mSawtoothButton(tr("Sawtooth")),
    mSineButton(tr("Sine")),
    mWaveramLayout(),
    mWaveramEdit(),
    mClearButton(tr("Clear")),
    mWaveEditedFromText(false)
{
    mPresetLayout.addWidget(&mSquare50Button);
    mPresetLayout.addWidget(&mSquare25Button);
    mPresetLayout.addWidget(&mTriangleButton);
    mPresetLayout.addWidget(&mSawtoothButton);
    mPresetLayout.addWidget(&mSineButton);

    mWaveramLayout.addWidget(&mWaveramEdit, 1);
    mWaveramLayout.addWidget(&mClearButton);

    mLayout.addWidget(&mGraph, 1);
    mLayout.addLayout(&mPresetLayout);
    mLayout.addLayout(&mWaveramLayout);
    mLayout.setMargin(0);

    auto &widget = editorWidget();
    widget.setLayout(&mLayout);

    mWaveramEdit.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mWaveramEdit.setMaxLength(32);
    mWaveramEdit.setAlignment(Qt::AlignCenter);
    mWaveramEdit.setInputMask(QStringLiteral("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"));
    mWaveramEdit.setText(QStringLiteral("00000000000000000000000000000000"));

    mGraph.setViewMode(GraphEdit::WaveformView);

    connect(&mWaveramEdit, &QLineEdit::textEdited, this, [this](QString const& text) {
        mWaveEditedFromText = true;
        mWaveModel.setDataFromString(text);
        mWaveEditedFromText = false;
    });
    connect(&mClearButton, &QPushButton::clicked, &mWaveModel, &WaveModel::clear);
    connect(&mWaveModel, &WaveModel::dataChanged, this, [this](){
        // update the waveram edit text if the change did not come from the waveram edit
        if (mWaveModel.count() && !mWaveEditedFromText) {
            mWaveramEdit.setText(mWaveModel.waveformToString());
        }
    });

    connect(&mSquare50Button, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_PULSE50);
    });
    connect(&mSquare25Button, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_PULSE25);
    });
    connect(&mTriangleButton, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_TRIANGLE);
    });
    connect(&mSawtoothButton, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_SAWTOOTH);
    });
    connect(&mSineButton, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_SINE);
    });

}

void WaveEditor::setCurrentItem(int index) {
    if (index == -1) {
        mWaveModel.setWaveform(nullptr, 0);
        mWaveramEdit.clear();
    } else {
        mWaveModel.setWaveform(document(), index);
        mWaveramEdit.setText(mWaveModel.waveformToString());
    }
}

BaseTableModel* WaveEditor::getModel(ModuleDocument &doc) {
    return &doc.waveModel();
}

void WaveEditor::setWaveFromPreset(int preset) {
    mWaveModel.setWaveformData(PRESETS[preset]);
}
