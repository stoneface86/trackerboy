
#include "core/model/WaveListModel.hpp"


WaveListModel::WaveListModel(ModuleDocument &document) :
    BaseTableModel(document, document.waveTable())
{
}

trackerboy::Waveform* WaveListModel::waveform(int modelIndex) {
    return static_cast<trackerboy::WaveTable&>(mBaseTable)[mBaseTable.lookup(static_cast<uint8_t>(modelIndex))];
}

trackerboy::Waveform* WaveListModel::currentWaveform() {
    return waveform(mCurrentIndex);
}

QVariant WaveListModel::iconData(const QModelIndex &index) const {
    (void)index;
    return QVariant();
}

void WaveListModel::setSample(QPoint point) {
    auto currentWaveform = waveform(mCurrentIndex);

    // sample index
    int index = point.x() / 2;
    auto data = currentWaveform->data();
    // get the samples
    uint8_t sample = data[index];
    if (!!(point.x() & 1)) {
        // even index, target sample is the lower nibble
        sample = (sample & 0xF0) | static_cast<uint8_t>(point.y());
    } else {
        // odd index, target sample is the upper nibble
        sample = (sample & 0x0F) | (static_cast<uint8_t>(point.y()) << 4);
    }
    
    {
        auto ctx = mDocument.beginEdit();
        data[index] = sample;
    }

    emit waveformChanged(point);
}

void WaveListModel::setData(uint8_t *data) {
    auto curData = waveform(mCurrentIndex)->data();
    {
        auto ctx = mDocument.beginEdit();
        for (int i = 0; i != 32; i += 2) {
            *curData++ = (data[i] << 4) | (data[i + 1]);
        }
    }
    
    emit waveformChanged();
}

void WaveListModel::setData(const QString &text) {
    {
        auto ctx = mDocument.beginEdit();
        waveform(mCurrentIndex)->fromString(text.toStdString());
    }

    emit waveformChanged();
}

void WaveListModel::clear() {
    auto currentWaveform = waveform(mCurrentIndex);

    {
        auto ctx = mDocument.beginEdit();
        std::fill_n(currentWaveform->data(), trackerboy::Gbs::WAVE_RAMSIZE, 0);
    }

    emit waveformChanged();
}


