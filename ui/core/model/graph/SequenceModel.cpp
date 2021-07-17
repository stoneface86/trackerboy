
#include "core/model/graph/SequenceModel.hpp"

SequenceModel::SequenceModel(QObject *parent) :
    GraphModel(parent),
    mModule(nullptr),
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
        auto ctx = mModule->permanentEdit();
        mSequence->data()[index] = data;
    }

    emit dataChanged();
}

void SequenceModel::setSize(int size) {
    if (count() != size) {
        {
            auto ctx = mModule->permanentEdit();
            mSequence->resize((size_t)size);
        }
        emit countChanged(size);
    }
}

void SequenceModel::setSequence(Module *mod, trackerboy::Sequence *seq) {
    if (mSequence == seq) {
        return;
    }

    auto curCount = count();
    mModule = mod;
    mSequence = seq;
    emit dataChanged();

    auto newCount = count();
    if (curCount != newCount) {
        emit countChanged(newCount);
    }

}

void SequenceModel::removeSequence() {
    bool hadSequence = mSequence != nullptr;
    if (hadSequence) {
        auto _count = count();
        mModule = nullptr;
        mSequence = nullptr;
        emit dataChanged();
        if (_count != 0) {
            emit countChanged(0);
        }
    }
}

trackerboy::Sequence* SequenceModel::sequence() const {
    return mSequence;
}

void SequenceModel::replaceData(std::vector<uint8_t> const& data) {
    size_t oldsize;
    {
        auto ctx = mModule->permanentEdit();
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
            auto ctx = mModule->permanentEdit();
            mSequence->setLoop(loop);
        }
    }
}

void SequenceModel::removeLoop() {
    if (mSequence->loop()) {
        {
            auto ctx = mModule->permanentEdit();
            mSequence->removeLoop();
        }
    }
}
