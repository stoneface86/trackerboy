
#include "model/WaveListModel.hpp"

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
    
    mDocument.lock();
    data[index] = sample;
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged(point);
}

void WaveListModel::setData(uint8_t *data) {
    auto curData = waveform(mCurrentIndex)->data();
    mDocument.lock();
    for (int i = 0; i != 32; i += 2) {
        *curData++ = (data[i] << 4) | (data[i + 1]);
    }
    mDocument.unlock();

    mDocument.setModified(true);
    
    emit waveformChanged();
}

void WaveListModel::setData(const QString &text) {
    mDocument.lock();
    waveform(mCurrentIndex)->fromString(text.toStdString());
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged();
}

void WaveListModel::clear() {
    auto currentWaveform = waveform(mCurrentIndex);

    mDocument.lock();
    std::fill_n(currentWaveform->data(), trackerboy::Gbs::WAVE_RAMSIZE, 0);
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged();
}

void WaveListModel::invert() {
    auto currentWaveform = waveform(mCurrentIndex);

    auto data = currentWaveform->data();

    mDocument.lock();
    for (int i = 0; i != trackerboy::Gbs::WAVE_RAMSIZE; ++i) {
        uint8_t sample = ~(*data);
        *data++ = sample;
    }
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged();
}

void WaveListModel::rotateLeft() {
    auto currentWaveform = waveform(mCurrentIndex);
    auto data = currentWaveform->data();

    // save the first sample for later
    
    uint8_t sample = data[0];
    uint8_t sampleFirst = sample >> 4;

    mDocument.lock();
    for (size_t i = 0; i != trackerboy::Gbs::WAVE_RAMSIZE - 1; ++i) {

        uint8_t next = data[i + 1];

        data[i] = (sample << 4) | (next >> 4);

        sample = next;
    }
    // wrap-around
    data[15] = sampleFirst | (data[15] << 4);
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged();
}

void WaveListModel::rotateRight() {
    auto currentWaveform = waveform(mCurrentIndex);
    auto data = currentWaveform->data();


    uint8_t prev = data[15];
    mDocument.lock();
    for (size_t i = 0; i != trackerboy::Gbs::WAVE_RAMSIZE - 1; ++i) {
        uint8_t sample = data[i];
        data[i] = (sample >> 4) | (prev << 4);
        prev = sample;
    }
    // wrap-around
    data[15] = (prev << 4) | (data[15] >> 4);
    mDocument.unlock();

    mDocument.setModified(true);

    emit waveformChanged();
}


