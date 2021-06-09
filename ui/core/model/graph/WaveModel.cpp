
#include "core/model/graph/WaveModel.hpp"

#include "core/model/ModuleDocument.hpp"
#include "trackerboy/data/Waveform.hpp"

WaveModel::WaveModel(QObject *parent) :
    GraphModel(parent),
    mDocument(nullptr),
    mWaveform(nullptr)
{
}

int WaveModel::count() {
    return (mWaveform) ? 32 : 0; // waveforms are always 32 samples
}

struct WaveIndex {

    WaveIndex(int i) :
        index(i >> 1),
        isLowNibble(i & 1)
    {
    }

    int index;
    bool isLowNibble;

};

WaveModel::DataType WaveModel::dataAt(int i) {
    WaveIndex wi(i);

    auto samplepair = mWaveform->operator[](wi.index);
    if (wi.isLowNibble) {
        return (DataType)(samplepair & 0xF);
    } else {
        return (DataType)(samplepair >> 4);
    }
}

void WaveModel::setData(int i, DataType data) {
    WaveIndex wi(i);

    {
        auto ctx = mDocument->beginEdit();
        auto &samplepairRef = mWaveform->operator[](wi.index);
        auto samplepair = samplepairRef;
        if (wi.isLowNibble) {
            samplepair &= 0xF0;
            samplepair |= data;
        } else {
            samplepair &= 0x0F;
            samplepair |= data << 4;
        }
        samplepairRef = samplepair;
    }

    emit dataChanged();

}

void WaveModel::setWaveform(ModuleDocument *doc, int waveIndex) {
    mDocument = doc;
    auto oldwave = mWaveform;
    if (doc) {
        auto &model = doc->waveModel();
        mWaveform = doc->mod().waveformTable().get(model.id(waveIndex));
    } else {
        mWaveform = nullptr;
    }

    if (oldwave != mWaveform) {
        emit dataChanged();
        if (oldwave == nullptr || mWaveform == nullptr) {
            emit countChanged(count());
        }
    }
}

void WaveModel::setWaveformData(trackerboy::Waveform::Data const& data) {
    {
        auto ctx = mDocument->beginEdit();
        std::copy(data.begin(), data.end(), mWaveform->data().begin());
    }

    emit dataChanged();
}

void WaveModel::setDataFromString(QString const& str) {
    if (mDocument) {
        {
            auto ctx = mDocument->beginEdit();
            mWaveform->fromString(str.toStdString());
        }
        emit dataChanged();
    }
}

QString WaveModel::waveformToString() {
    auto str = mWaveform->toString();
    return QString::fromStdString(str);
}

void WaveModel::clear() {
    {
        auto ctx = mDocument->beginEdit();
        mWaveform->data().fill((uint8_t)0);
    }
    emit dataChanged();
}
