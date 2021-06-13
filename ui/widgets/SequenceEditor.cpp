
#include "widgets/SequenceEditor.hpp"

#include <QtDebug>
#include <QElapsedTimer>

#include <algorithm>

//#define PROFILE_STRING_CONVERSION

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

    #ifdef PROFILE_STRING_CONVERSION
    QElapsedTimer timer;
    timer.start();
    #endif

    mEditDirty = false;

    auto const str = mSequenceInput.text();
    auto tokens = str.splitRef(' ');
    std::vector<uint8_t> newdata;

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
                    newdata.push_back((uint8_t)num);
                    if (++index == (int)trackerboy::Sequence::MAX_SIZE) {
                        break;
                    }
                }
            }
        } 
    }

    #ifdef PROFILE_STRING_CONVERSION
    qDebug() << "Converted string to sequence in" << timer.nsecsElapsed() / 1000 << "microseconds";
    #endif

    mIgnoreUpdates = true;
    mModel.replaceData(newdata);
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

    // profiling notes:
    // Windows 10, Ryzen 5 2600
    // converting the maximum size sequence, 256, took 1000-1200 microseconds
    // small sequences 0-10 take about 20-40 microseconds
    // string to sequence conversion is more than twice as fast

    #ifdef PROFILE_STRING_CONVERSION
    QElapsedTimer timer;
    timer.start();
    #endif

    auto sequence = mModel.sequence();

    if (sequence) {
        auto const& sequenceData = sequence->data();

        auto size = sequenceData.size();
        if (size) {
            auto loop = sequence->loop();
            int loopIndex = loop ? *loop : -1;
            int index = 0;
            
            QString str;
            // reserve helps reduce the number of allocations that occur when appending
            // guess the final size of the string by multiplying the size of the sequence
            str.reserve((int)(size * 2));

            for (auto value : sequenceData) {
                if (index++ == loopIndex) {
                    str.append(QStringLiteral("| "));
                }
                str.append(QString::number((int8_t)value));
                str.append(' ');
            }
            str.chop(1);
            mSequenceInput.setText(str);
        } else {
            mSequenceInput.clear();
        }
    } else {
        mSequenceInput.clear();
    }

    #ifdef PROFILE_STRING_CONVERSION
    qDebug() << "Converted sequence to string in" << timer.nsecsElapsed() / 1000 << "microseconds";
    #endif

}
