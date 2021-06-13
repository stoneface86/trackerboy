
#include "widgets/SequenceEditor.hpp"

#include <QtDebug>

#include <algorithm>

SequenceEditor::SequenceEditor(size_t sequenceIndex, QWidget *parent) :
    QWidget(parent),
    mModel(),
    mSequenceIndex(sequenceIndex),
    mLayout(),
    mGraph(mModel),
    mSizeLayout(),
    mSizeLabel(tr("Size")),
    mSizeSpin(),
    mSequenceInput(),
    mNumberScratch(4),
    mStringScratch(),
    mIgnoreUpdates(false),
    mEditDirty(false)
{
    switch (sequenceIndex) {
        case trackerboy::Instrument::SEQUENCE_ARP:
            mGraph.setViewMode(GraphEdit::ArpeggioView);
            break;
        case trackerboy::Instrument::SEQUENCE_PANNING:
            mGraph.setViewMode(GraphEdit::PanningView);
            break;
        case trackerboy::Instrument::SEQUENCE_PITCH:
            mGraph.setViewMode(GraphEdit::PitchView);
            break;
        case trackerboy::Instrument::SEQUENCE_TIMBRE:
            mGraph.setViewMode(GraphEdit::TimbreView);
            break;
        default:
            qFatal("Invalid sequence parameter given");
    }

    mSizeLayout.addWidget(&mSizeLabel);
    mSizeLayout.addWidget(&mSizeSpin);
    mSizeLayout.addWidget(&mSequenceInput, 1);

    mLayout.addWidget(&mGraph, 1);
    mLayout.addLayout(&mSizeLayout);
    setLayout(&mLayout);

    mSizeSpin.setRange(0, (int)trackerboy::Sequence::MAX_SIZE);

    connect(&mSizeSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SequenceModel::setSize);
    connect(&mModel, &SequenceModel::countChanged, &mSizeSpin, &QSpinBox::setValue);

    connect(&mModel, &SequenceModel::dataChanged, this, &SequenceEditor::updateString);
    connect(&mModel, &SequenceModel::countChanged, this, &SequenceEditor::updateString);
    connect(&mSequenceInput, &QLineEdit::editingFinished, this, &SequenceEditor::convertEditToSequence);
    connect(&mSequenceInput, &QLineEdit::textEdited, this,
        [this]() {
            mEditDirty = true;
        });
}

void SequenceEditor::setInstrument(ModuleDocument *doc, trackerboy::Instrument *instrument) {
    if (instrument && doc) {
        mModel.setSequence(doc, &instrument->sequence(mSequenceIndex));
    } else {
        removeInstrument();
    }
}

void SequenceEditor::removeInstrument() {
    mModel.removeSequence();
}

void SequenceEditor::convertEditToSequence() {
    if (!mEditDirty) {
        return;
    }

    mEditDirty = false;

    auto const str = mSequenceInput.text();
    auto tokens = str.splitRef(' ');
    mSequenceDataScratch.clear();

    auto min = mGraph.minimumValue();
    auto max = mGraph.maximumValue();

    size_t index = 0;
    std::optional<uint8_t> loopIndex;

    for (auto token : tokens) {
        if (token.size() > 0) {
            if (token[0] == '|') {
                // set loop index
                loopIndex = (uint8_t)index;
            } else {
                bool ok;
                auto num = token.toInt(&ok);
                if (ok) {
                    num = std::clamp(num, min, max);
                    mSequenceDataScratch.push_back((uint8_t)num);
                    if (++index == (int)trackerboy::Sequence::MAX_SIZE) {
                        break;
                    }
                }
            }
        } 
    }

    mIgnoreUpdates = true;
    mModel.replaceData(mSequenceDataScratch);
    if (loopIndex) {
        mModel.setLoop(*loopIndex);
    } else {
        mModel.removeLoop();
    }
    mIgnoreUpdates = false;
    updateString();
}

void SequenceEditor::updateString() {
    if (mIgnoreUpdates) {
        return;
    }

    auto sequence = mModel.sequence();

    if (sequence) {
        auto const& sequenceData = sequence->data();
        auto loop = sequence->loop();
        int loopIndex = loop ? *loop : -1;
        int index = 0;

        mStringScratch.clear();
        for (auto value : sequenceData) {
            if (index++ == loopIndex) {
                mStringScratch.append(QStringLiteral("| "));
            }
            mNumberScratch.setNum((int8_t)value);
            mStringScratch.append(mNumberScratch);
            mStringScratch.append(' ');
        }
        mStringScratch.chop(1);
        mSequenceInput.setText(mStringScratch);
    } else {
        mSequenceInput.clear();
    }
}
