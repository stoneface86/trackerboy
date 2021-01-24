
#include "core/model/InstrumentListModel.hpp"
#include "misc/IconManager.hpp"


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

    {
        auto ctx = mDocument.beginEdit();
    inst->data().channel = static_cast<uint8_t>(ch);
    }

    dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DecorationRole });
}

void InstrumentListModel::setPanning(uint8_t panning) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().panning = panning;
    }
}

void InstrumentListModel::setDelay(uint8_t delay) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().delay = delay;
    }
}

void InstrumentListModel::setDuration(uint8_t duration) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().duration = duration;
    }
}

void InstrumentListModel::setTune(int8_t tune) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().tune = tune;
    }
}

void InstrumentListModel::setEnvelope(uint8_t envelope) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().envelope = envelope;
    }
}

void InstrumentListModel::setTimbre(uint8_t timbre) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().timbre = timbre;
    }
}

void InstrumentListModel::setVibrato(uint8_t extent, uint8_t speed) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().vibrato = (speed << 4) | extent;
    }
}

void InstrumentListModel::setVibratoDelay(uint8_t delay) {
    auto inst = instrument(mCurrentIndex);
    {
        auto ctx = mDocument.beginEdit();
        inst->data().vibratoDelay = delay;
    }
}


QVariant InstrumentListModel::iconData(const QModelIndex &index) const {
    auto inst = instrument(index.row());
    return *mIconArray[inst->data().channel];
}


