
#pragma once

#include "core/model/graph/SequenceModel.hpp"
#include "widgets/GraphEdit.hpp"

#include "trackerboy/data/Instrument.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QVBoxLayout>


class SequenceEditor : public QWidget {

    Q_OBJECT

public:
    explicit SequenceEditor(size_t sequenceIndex, QWidget *parent = nullptr);

    void setInstrument(ModuleDocument *doc, trackerboy::Instrument *instrument);

    void removeInstrument();

private:
    void convertEditToSequence();

    void updateString();

    Q_DISABLE_COPY(SequenceEditor)


    SequenceModel mModel;
    size_t const mSequenceIndex;

    QVBoxLayout mLayout;
        GraphEdit mGraph;
        QHBoxLayout mSizeLayout;
            QLabel mSizeLabel;
            QSpinBox mSizeSpin;
            QLineEdit mSequenceInput;

    bool mIgnoreUpdates;
    bool mEditDirty;

};
