

#include "model/InstrumentListModel.hpp"

InstrumentListModel::InstrumentListModel(trackerboy::InstrumentTable &table, QObject *parent) :
    BaseTableModel(table, parent)
{
}

trackerboy::Instrument* InstrumentListModel::instrument(int modelIndex) {
    return static_cast<trackerboy::InstrumentTable&>(mBaseTable)[mBaseTable.lookup(modelIndex)];
}

QVariant InstrumentListModel::iconData(const QModelIndex &index) const {
    return QVariant();
}


