
#pragma once

class ModuleDocument;

#include "core/model/graph/GraphModel.hpp"

#include "trackerboy/data/Sequence.hpp"


class SequenceModel : public GraphModel {

    Q_OBJECT

public:
    explicit SequenceModel(QObject *parent = nullptr);

    virtual int count() override;

    virtual DataType dataAt(int index) override;

    virtual void setData(int index, DataType data) override;

    void setSequence(ModuleDocument *doc, trackerboy::Sequence *seq);

    void removeSequence();

    void setSize(int size);

    void setLoop(uint8_t pos);

    void removeLoop();

    void replaceData(std::vector<uint8_t> const& data);

    trackerboy::Sequence* sequence() const;

private:
    ModuleDocument *mDocument;
    trackerboy::Sequence *mSequence;

};
