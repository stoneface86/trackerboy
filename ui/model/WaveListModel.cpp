
#include "model/WaveListModel.hpp"

WaveListModel::WaveListModel(trackerboy::WaveTable &table, QObject *parent) :
    mTable(table),
    BaseTableModel(table, parent)
{
}

trackerboy::Waveform* WaveListModel::waveform(int modelIndex) {
    auto iter = mBaseTable.begin() + modelIndex;
    return static_cast<trackerboy::WaveTable&>(mBaseTable)[iter->index];
}

QVariant WaveListModel::iconData(const QModelIndex &index) const {
    return QVariant();
}

void WaveListModel::insertItemInTable() {
    static_cast<trackerboy::WaveTable&>(mTable).insert();
}

void WaveListModel::removeItemInTable(uint8_t id) {

}

