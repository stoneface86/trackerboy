
#include "model/BaseModel.hpp"


BaseModel::BaseModel(QObject *parent) :
    mCurrentIndex(-1),
    QAbstractListModel(parent)
{
}

int BaseModel::currentIndex() const {
    return mCurrentIndex;
}

void BaseModel::select(int index) {
    if (mCurrentIndex != index) {
        mCurrentIndex = index;
        emit currentIndexChanged(index);
    }
}

void BaseModel::select(const QModelIndex &index) {
    select(index.row());
}

void BaseModel::setEnabled(bool enabled) {
    if (enabled) {
        endResetModel();
    } else {
        beginResetModel();
    }
}

void BaseModel::setName(QString name) {
    setNameInData(name);
    emit dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DisplayRole });
}
