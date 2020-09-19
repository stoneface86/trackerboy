
#include "model/BaseModel.hpp"


BaseModel::BaseModel(ModuleDocument &document) :
    mDocument(document),
    mCurrentIndex(-1),
    mActNew(nullptr),
    mActRemove(nullptr),
    mActDuplicate(nullptr),
    mActEdit(nullptr),
    QAbstractListModel(&document)
{
}

int BaseModel::currentIndex() const {
    return mCurrentIndex;
}

void BaseModel::add() {
    select(dataAdd());
}

void BaseModel::remove() {
    select(dataRemove());
}

void BaseModel::duplicate() {
    select(dataDuplicate());
}

void BaseModel::rename(const QString &name) {
    dataRename(name);
    emit dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DisplayRole });
}

void BaseModel::select(int index) {
    if (mCurrentIndex != index) {
        mCurrentIndex = index;
        emit currentIndexChanged(index);

        bool hasSelection = index != -1;
        if (mActRemove != nullptr) {
            mActRemove->setEnabled(hasSelection && canRemove());
        }

        if (mActDuplicate != nullptr) {
            mActDuplicate->setEnabled(hasSelection && canDuplicate());
        }

        if (mActEdit != nullptr) {
            mActEdit->setEnabled(hasSelection);
        }
    }
    
}

void BaseModel::select(const QModelIndex &index) {
    select(index.row());
}

void BaseModel::setEnabled(bool enabled) {
    if (enabled) {
        endResetModel();
        if (rowCount() > 0) {
            select(0);
        }
    } else {
        beginResetModel();
        select(-1);
    }
}

void BaseModel::setActions(QAction *actNew, QAction *actRemove, QAction *actDuplicate, QAction *actEdit) {
    mActNew = actNew;
    mActRemove = actRemove;
    mActDuplicate = actDuplicate;
    mActEdit = actEdit;
}

