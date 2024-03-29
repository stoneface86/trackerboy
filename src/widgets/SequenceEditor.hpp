
#pragma once

#include "core/Module.hpp"
#include "model/graph/SequenceModel.hpp"
#include "widgets/GraphEdit.hpp"

#include "trackerboy/data/Instrument.hpp"

#include <QLineEdit>
#include <QWidget>


class SequenceEditor : public QWidget {

    Q_OBJECT

public:
    SequenceEditor(
        Module &mod,
        size_t sequenceIndex,
        QWidget *parent = nullptr
    );

    void setInstrument(trackerboy::Instrument *instrument);

    GraphEdit* graphEdit();

private:
    void convertEditToSequence();

    void updateString();

    Q_DISABLE_COPY(SequenceEditor)


    SequenceModel *mModel;
    size_t const mSequenceIndex;

    GraphEdit *mGraph;
    QLineEdit *mSequenceInput;

    bool mIgnoreUpdates;
    bool mEditDirty;

};
