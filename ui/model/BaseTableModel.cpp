
#include "model/BaseTableModel.hpp"


BaseTableModel::BaseTableModel(trackerboy::BaseTable &table, QObject *parent) :
    mBaseTable(table),
    //mEnabled(true),
    BaseModel(parent)
{
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return static_cast<int>(mBaseTable.size());
    //return (mEnabled) ? mBaseTable.size() : 0;
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    //if (mEnabled) {
        if (role == Qt::DisplayRole) {
            auto *item = mBaseTable.getFromOrder(static_cast<uint8_t>(index.row()));
            QString str("%1 - %2");
            return QVariant(str.arg(
                QString::number(item->id(), 16).toUpper().rightJustified(2, '0'), 
                QString::fromStdString(item->name())
            ));
        } else if (role == Qt::DecorationRole) {
            return iconData(index);
        }
    //}

    return QVariant();
}

void BaseTableModel::addItem() {
    int row = static_cast<int>(mBaseTable.size());
    beginInsertRows(QModelIndex(), row, row);
    mBaseTable.insertItem();
    endInsertRows();
}

QString BaseTableModel::name() {
    return QString::fromStdString(mBaseTable.getFromOrder(static_cast<uint8_t>(mCurrentIndex))->name());
}

void BaseTableModel::setNameInData(QString name) {
    mBaseTable.getFromOrder(static_cast<uint8_t>(mCurrentIndex))->setName(name.toStdString());
}
