
#include "core/model/graph/SequenceModel.hpp"
#include "core/model/ModuleDocument.hpp"


SequenceModel::SequenceModel(QObject *parent) :
    GraphModel(parent),
    mDocument(nullptr),
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
        auto ctx = mDocument->beginEdit();
        mSequence->data()[index] = data;
    }

    emit dataChanged();
}

void SequenceModel::setSize(int size) {
    if (count() != size) {
        {
            auto ctx = mDocument->beginEdit();
            mSequence->resize((size_t)size);
        }
        emit countChanged(size);
    }
}

void SequenceModel::setSequence(ModuleDocument *doc, trackerboy::Sequence *seq) {
    if (mSequence == seq) {
        return;
    }

    auto curCount = count();
    mDocument = doc;
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
        mDocument = nullptr;
        mSequence = nullptr;
        emit dataChanged();
        if (_count != 0) {
            emit countChanged(0);
        }
    }
}

