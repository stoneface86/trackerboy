

#include "model/InstrumentListModel.hpp"

InstrumentListModel::InstrumentListModel(trackerboy::InstrumentTable &table, QObject *parent) :
    mPulseIcon(":/icons/instrument_icon_pulse.png"),
    mWaveIcon(":/icons/instrument_icon_wave.png"),
    mNoiseIcon(":/icons/instrument_icon_noise.png"),
    BaseTableModel(table, parent)
{
}

trackerboy::Instrument* InstrumentListModel::instrument(int modelIndex) const {
    return static_cast<trackerboy::InstrumentTable&>(mBaseTable)[mBaseTable.lookup(modelIndex)];
}

QVariant InstrumentListModel::iconData(const QModelIndex &index) const {
    auto inst = instrument(index.row());

    trackerboy::ChType ch = static_cast<trackerboy::ChType>(inst->data().channel);
    switch (ch) {
        case trackerboy::ChType::ch1:
        case trackerboy::ChType::ch2:
            return QVariant(mPulseIcon);
        case trackerboy::ChType::ch3:
            return QVariant(mWaveIcon);
        case trackerboy::ChType::ch4:
            return QVariant(mNoiseIcon);
    }
    return QVariant();
}

