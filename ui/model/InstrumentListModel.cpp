

#include "misc/IconManager.hpp"
#include "model/InstrumentListModel.hpp"

InstrumentListModel::InstrumentListModel(ModuleDocument &document) :
    BaseTableModel(document, document.instrumentTable())
{
    size_t iconId = static_cast<size_t>(Icons::ch1);
    for (int i = 0; i != 4; ++i) {
        mIconArray[i] = &IconManager::getIcon(static_cast<Icons>(iconId));
        ++iconId;
    }

}

trackerboy::Instrument* InstrumentListModel::instrument(int modelIndex) const {
    return static_cast<trackerboy::InstrumentTable&>(mBaseTable)[mBaseTable.lookup(static_cast<uint8_t>(modelIndex))];
}

void InstrumentListModel::setChannel(trackerboy::ChType ch) {
    auto inst = instrument(mCurrentIndex);

    mDocument.lock();
    inst->data().channel = static_cast<uint8_t>(ch);
    mDocument.unlock();

    dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DecorationRole });
}

void InstrumentListModel::setPanning(uint8_t panning) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().panning = panning;
    mDocument.unlock();
}

void InstrumentListModel::setDelay(uint8_t delay) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().delay = delay;
    mDocument.unlock();
}

void InstrumentListModel::setDuration(uint8_t duration) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().duration = duration;
    mDocument.unlock();
}

void InstrumentListModel::setTune(int8_t tune) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().tune = tune;
    mDocument.unlock();
}

void InstrumentListModel::setEnvelope(uint8_t envelope) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().envelope = envelope;
    mDocument.unlock();
}

void InstrumentListModel::setTimbre(uint8_t timbre) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().timbre = timbre;
    mDocument.unlock();
}

void InstrumentListModel::setVibrato(uint8_t extent, uint8_t speed) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().vibrato = (speed << 4) | extent;
    mDocument.unlock();
}

void InstrumentListModel::setVibratoDelay(uint8_t delay) {
    auto inst = instrument(mCurrentIndex);
    mDocument.lock();
    inst->data().vibratoDelay = delay;
    mDocument.unlock();
}


QVariant InstrumentListModel::iconData(const QModelIndex &index) const {
    auto inst = instrument(index.row());
    return *mIconArray[inst->data().channel];
}


