
#include "model/graph/WaveModel.hpp"

WaveModel::WaveModel(Module &mod, QObject *parent) :
    GraphModel(mod, parent),
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
        auto ctx = mModule.permanentEdit();
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

void WaveModel::setWaveform(trackerboy::Waveform *waveform) {
    auto oldwave = mWaveform;
    mWaveform = waveform;

    if (oldwave != mWaveform) {
        emit dataChanged();
        if (oldwave == nullptr || mWaveform == nullptr) {
            emit countChanged(count());
        }
    }
}

void WaveModel::setWaveformData(trackerboy::Waveform::Data const& data) {
    {
        auto ctx = mModule.permanentEdit();
        std::copy(data.begin(), data.end(), mWaveform->data().begin());
    }

    emit dataChanged();
}

void WaveModel::setDataFromString(QString const& str) {
 
    {
        auto ctx = mModule.permanentEdit();
        mWaveform->fromString(str.toStdString());
    }
    emit dataChanged();
}

QString WaveModel::waveformToString() {
    auto str = mWaveform->toString();
    return QString::fromStdString(str);
}

void WaveModel::clear() {
    {
        auto ctx = mModule.permanentEdit();
        mWaveform->data().fill((uint8_t)0);
    }
    emit dataChanged();
}
