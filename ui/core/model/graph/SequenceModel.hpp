
#pragma once

#include "core/Module.hpp"
#include "core/model/graph/GraphModel.hpp"

#include "trackerboy/data/Sequence.hpp"


class SequenceModel : public GraphModel {

    Q_OBJECT

public:
    explicit SequenceModel(QObject *parent = nullptr);

    virtual int count() override;

    virtual DataType dataAt(int index) override;

    virtual void setData(int index, DataType data) override;

    void setSequence(Module *mod, trackerboy::Sequence *seq);

    void removeSequence();

    void setSize(int size);

    void setLoop(uint8_t pos);

    void removeLoop();

    void replaceData(std::vector<uint8_t> const& data);

    trackerboy::Sequence* sequence() const;

private:
    Module *mModule;
    trackerboy::Sequence *mSequence;

};
