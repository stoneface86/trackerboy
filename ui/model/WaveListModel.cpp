
#include "model/WaveListModel.hpp"

WaveListModel::WaveListModel(trackerboy::WaveTable &table, QObject *parent) :
    mTable(table),
    BaseTableModel(table, parent)
{
}

trackerboy::Waveform* WaveListModel::waveform(int modelIndex) {
    return mTable[mTable.lookup(modelIndex)];
}

QVariant WaveListModel::iconData(const QModelIndex &index) const {
    return QVariant();
}


