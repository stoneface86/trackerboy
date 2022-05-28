
#include "widgets/SequenceEditor.hpp"
#include "verdigris/wobjectimpl.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QtDebug>
#include <QVBoxLayout>

#ifdef PROFILE_STRING_CONVERSION
#include <QElapsedTimer>
#endif

#include <algorithm>

//#define PROFILE_STRING_CONVERSION

W_OBJECT_IMPL(SequenceEditor)

SequenceEditor::SequenceEditor(
    Module &mod,
    size_t sequenceIndex,
    QWidget *parent
) :
    QWidget(parent),
    mModel(nullptr),
    mSequenceIndex(sequenceIndex),
    mGraph(nullptr),
    mSequenceInput(nullptr),
    mIgnoreUpdates(false),
    mEditDirty(false)
{
    mModel = new SequenceModel(mod, this);

    mGraph = new GraphEdit(*mModel);
    switch (sequenceIndex) {
        case trackerboy::Instrument::SEQUENCE_ARP:
            mGraph->setViewMode(GraphEdit::ArpeggioView);
            break;
        case trackerboy::Instrument::SEQUENCE_PANNING:
            mGraph->setViewMode(GraphEdit::PanningView);
            break;
        case trackerboy::Instrument::SEQUENCE_PITCH:
            mGraph->setViewMode(GraphEdit::PitchView);
            break;
        case trackerboy::Instrument::SEQUENCE_TIMBRE:
            mGraph->setViewMode(GraphEdit::TimbreView);
            break;
        default:
            qFatal("Invalid sequence parameter given");
    }

    auto layout = new QVBoxLayout;
    auto sizeLayout = new QHBoxLayout;
    auto sizeSpin = new QSpinBox;
    mSequenceInput = new QLineEdit;
    sizeLayout->addWidget(new QLabel(tr("Size")));
    sizeLayout->addWidget(sizeSpin);
    sizeLayout->addWidget(mSequenceInput, 1);
    layout->addWidget(mGraph, 1);
    layout->addLayout(sizeLayout);
    setLayout(layout);

    sizeSpin->setRange(0, (int)trackerboy::Sequence::MAX_SIZE);

    connect(sizeSpin, qOverload<int>(&QSpinBox::valueChanged), mModel, &SequenceModel::setSize);
    connect(mModel, &SequenceModel::countChanged, sizeSpin, &QSpinBox::setValue);

    connect(mModel, &SequenceModel::dataChanged, this, &SequenceEditor::updateString);
    connect(mModel, &SequenceModel::countChanged, this, &SequenceEditor::updateString);
    connect(mSequenceInput, &QLineEdit::editingFinished, this, &SequenceEditor::convertEditToSequence);
    connect(mSequenceInput, &QLineEdit::textEdited, this,
        [this]() {
            mEditDirty = true;
        });
}

void SequenceEditor::setInstrument(trackerboy::Instrument *instrument) {
    if (instrument) {
        mModel->setSequence(&instrument->sequence(mSequenceIndex));
    } else {
        mModel->setSequence(nullptr);
    }
}

GraphEdit* SequenceEditor::graphEdit() {
    return mGraph;
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

    auto const str = mSequenceInput->text();
    auto tokens = str.splitRef(' ');
    std::vector<uint8_t> newdata;

    auto min = mGraph->minimumValue();
    auto max = mGraph->maximumValue();

    size_t index = 0;
    std::optional<uint8_t> loopIndex;

    for (auto const& token : tokens) {
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
    mModel->replaceData(newdata);
    if (loopIndex) {
        mModel->setLoop(*loopIndex);
    } else {
        mModel->removeLoop();
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

    auto sequence = mModel->sequence();

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
            mSequenceInput->setText(str);
        } else {
            mSequenceInput->clear();
        }
    } else {
        mSequenceInput->clear();
    }

    #ifdef PROFILE_STRING_CONVERSION
    qDebug() << "Converted sequence to string in" << timer.nsecsElapsed() / 1000 << "microseconds";
    #endif

}
