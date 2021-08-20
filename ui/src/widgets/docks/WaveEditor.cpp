
#include "widgets/docks/WaveEditor.hpp"

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

WaveEditor::WaveEditor(
    Module &mod,
    WaveListModel &model,
    PianoInput const& input,
    QWidget *parent
) :
    BaseEditor(model, input, parent),
    mWaveModel(nullptr),
    mWaveramEdit(nullptr),
    mWaveEditedFromText(false),
    mWaveform(nullptr)
{

    mWaveModel = new WaveModel(mod, this);

    auto presetLayout = new QHBoxLayout;
    auto square50btn = new QPushButton(tr("50% Duty"));
    auto square25btn = new QPushButton(tr("25% Duty"));
    auto trianglebtn = new QPushButton(tr("Triangle"));
    auto sawbtn = new QPushButton(tr("Sawtooth"));
    auto sinebtn = new QPushButton(tr("Sine"));
    presetLayout->addWidget(square50btn);
    presetLayout->addWidget(square25btn);
    presetLayout->addWidget(trianglebtn);
    presetLayout->addWidget(sawbtn);
    presetLayout->addWidget(sinebtn);

    auto waveramLayout = new QHBoxLayout;
    mWaveramEdit = new QLineEdit;
    auto clearbtn = new QPushButton(tr("Clear"));
    waveramLayout->addWidget(mWaveramEdit, 1);
    waveramLayout->addWidget(clearbtn);



    auto layout = new QVBoxLayout;
    auto graph = new GraphEdit(*mWaveModel);
    layout->addWidget(graph, 1);
    layout->addLayout(presetLayout);
    layout->addLayout(waveramLayout);
    layout->setMargin(0);
    editorWidget()->setLayout(layout);

    mWaveramEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mWaveramEdit->setMaxLength(32);
    mWaveramEdit->setAlignment(Qt::AlignCenter);
    mWaveramEdit->setInputMask(QStringLiteral("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"));
    mWaveramEdit->setText(QStringLiteral("00000000000000000000000000000000"));

    graph->setViewMode(GraphEdit::WaveformView);

    connect(mWaveramEdit, &QLineEdit::textEdited, this,
        [this](QString const& text) {
            mWaveEditedFromText = true;
            mWaveModel->setDataFromString(text);
            mWaveEditedFromText = false;
        });
    connect(clearbtn, &QPushButton::clicked, mWaveModel, &WaveModel::clear);
    connect(mWaveModel, &WaveModel::dataChanged, this, [this](){
        // update the waveram edit text if the change did not come from the waveram edit
        if (mWaveModel->count() && !mWaveEditedFromText) {
            mWaveramEdit->setText(mWaveModel->waveformToString());
        }
    });

    connect(square50btn, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_PULSE50);
    });
    connect(square25btn, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_PULSE25);
    });
    connect(trianglebtn, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_TRIANGLE);
    });
    connect(sawbtn, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_SAWTOOTH);
    });
    connect(sinebtn, &QPushButton::clicked, this, [this](){
        setWaveFromPreset(PRESET_SINE);
    });

}

void WaveEditor::setCurrentItem(int index) {
    auto waveform = model().getShared(index);
    mWaveModel->setWaveform(waveform.get());
    // we keep a shared_ptr in this class so the wave model doesn't ever end
    // up with a dangling Waveform pointer
    mWaveform = std::move(waveform);

    if (index == -1) {
        mWaveramEdit->clear();
    } else {
        mWaveramEdit->setText(mWaveModel->waveformToString());
    }

}

WaveListModel& WaveEditor::model() const noexcept {
    // utility method, compiler should inline this
    return static_cast<WaveListModel&>(mModel);
}

void WaveEditor::setWaveFromPreset(int preset) {
    mWaveModel->setWaveformData(PRESETS[preset]);
}
