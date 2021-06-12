
#include "widgets/editors/InstrumentEditor.hpp"

#include <algorithm>

enum TabSequence {
    TabArpeggio,
    TabPanning,
    TabPitch,
    TabTimbre
};

static std::array const TAB_TO_SEQUENCE_INDEX = {
    trackerboy::Instrument::SEQUENCE_ARP,
    trackerboy::Instrument::SEQUENCE_PANNING,
    trackerboy::Instrument::SEQUENCE_PITCH,
    trackerboy::Instrument::SEQUENCE_TIMBRE
};

InstrumentEditor::InstrumentEditor(PianoInput const& input, QWidget *parent) :
    BaseEditor(input, tr("instrument"), parent),
    mSequenceModel(),
    mLayout(),
    mTabLayout(),
    mSequenceTabs(),
    mTabFrame(),
    mSequenceLayout(),
    mGraph(mSequenceModel),
    mSizeLayout(),
    mSizeLabel(tr("Size:")),
    mSizeSpin(),
    mInstrument(nullptr)
{


    mTabLayout.addWidget(&mSequenceTabs);
    mTabLayout.addWidget(&mTabFrame, 1);
    mTabLayout.setMargin(0);
    mTabLayout.setSpacing(0);

    mSequenceLayout.addWidget(&mGraph, 1);
    mSequenceLayout.addLayout(&mSizeLayout);
    mTabFrame.setLayout(&mSequenceLayout);

    mSizeLayout.addStretch();
    mSizeLayout.addWidget(&mSizeLabel);
    mSizeLayout.addWidget(&mSizeSpin);


    mLayout.addLayout(&mTabLayout);

    auto &_editorWidget = editorWidget();
    _editorWidget.setLayout(&mLayout);
    mTabFrame.setBackgroundRole(QPalette::Base);
    mTabFrame.setAutoFillBackground(true);
    

    mSequenceTabs.setDocumentMode(false);
    mSequenceTabs.setDrawBase(true);
    mSequenceTabs.addTab(tr("Arpeggio"));
    mSequenceTabs.addTab(tr("Panning"));
    mSequenceTabs.addTab(tr("Pitch"));
    mSequenceTabs.addTab(tr("Timbre"));

    mGraph.setViewMode(GraphEdit::ArpeggioView);

    connect(&mSizeSpin, qOverload<int>(&QSpinBox::valueChanged), &mSequenceModel, &SequenceModel::setSize);
    connect(&mSequenceModel, &SequenceModel::countChanged, &mSizeSpin, &QSpinBox::setValue);

    connect(&mSequenceTabs, &QTabBar::currentChanged, this,
        [this](int index){
            switch (index) {
                case TabArpeggio:
                    mGraph.setViewMode(GraphEdit::ArpeggioView);
                    break;
                case TabPanning:
                    mGraph.setViewMode(GraphEdit::PanningView);
                    break;
                case TabPitch:
                    mGraph.setViewMode(GraphEdit::PitchView);
                    break;
                case TabTimbre:
                    mGraph.setViewMode(GraphEdit::TimbreView);
                    break;
                default:
                    return;
            }

            setSequenceInModel(index);
        });

}

void InstrumentEditor::setCurrentItem(int index) {
    auto doc = document();
    if (index == -1) {
        mSequenceModel.removeSequence();
        mInstrument = nullptr;
    } else {
        mInstrument = doc->mod().instrumentTable().get(doc->instrumentModel().id(index));
        setSequenceInModel(mSequenceTabs.currentIndex());
        
    }

}

BaseTableModel* InstrumentEditor::getModel(ModuleDocument &doc) {
    return &doc.instrumentModel();
}

void InstrumentEditor::setSequenceInModel(int index) {
    mSequenceModel.setSequence(document(), &mInstrument->sequence(TAB_TO_SEQUENCE_INDEX[index]));
}
