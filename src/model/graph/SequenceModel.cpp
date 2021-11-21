
#include "model/graph/SequenceModel.hpp"

SequenceModel::SequenceModel(Module &mod, QObject *parent) :
    GraphModel(mod, parent),
    mSequence(nullptr)
{
}

int SequenceModel::count() {
    return (mSequence) ? (int)mSequence->data().size() : 0;
}

SequenceModel::DataType SequenceModel::dataAt(int index) {
    return mSequence->data()[index];
}

void SequenceModel::setData(int index, DataType data) {
    {
        auto ctx = mModule.permanentEdit();
        mSequence->data()[index] = data;
    }

    emit dataChanged();
}

void SequenceModel::setSize(int size) {
    if (count() != size) {
        {
            auto ctx = mModule.permanentEdit();
            mSequence->resize((size_t)size);
        }
        emit countChanged(size);
    }
}

void SequenceModel::setSequence(trackerboy::Sequence *seq) {
    if (mSequence == seq) {
        return;
    }

    auto curCount = count();
    mSequence = seq;
    emit dataChanged();

    auto newCount = count();
    if (curCount != newCount) {
        emit countChanged(newCount);
    }

}

trackerboy::Sequence* SequenceModel::sequence() const {
    return mSequence;
}

void SequenceModel::replaceData(std::vector<uint8_t> const& data) {
    size_t oldsize;
    {
        auto ctx = mModule.permanentEdit();
        auto &seqdata = mSequence->data();
        oldsize = seqdata.size();
        seqdata = data;
    }

    emit dataChanged();

    if (oldsize != data.size()) {
        emit countChanged((int)data.size());
    }
}

void SequenceModel::setLoop(uint8_t loop) {
    if (mSequence->loop() != loop) {
        {
            auto ctx = mModule.permanentEdit();
            mSequence->setLoop(loop);
        }
    }
}

void SequenceModel::removeLoop() {
    if (mSequence->loop()) {
        {
            auto ctx = mModule.permanentEdit();
            mSequence->removeLoop();
        }
    }
}
