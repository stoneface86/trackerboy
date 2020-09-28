
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
    int row = nextIndex();
    beginInsertRows(QModelIndex(), row, row);

    mDocument.lock();
    dataAdd();
    mDocument.unlock();

    endInsertRows();
    // select the newly inserted row
    select(row);
    mDocument.setModified(true);
}

void BaseModel::remove() {
    int row = mCurrentIndex;
    
    beginRemoveRows(QModelIndex(), row, row);

    mDocument.lock();
    dataRemove(row);
    mDocument.unlock();

    endRemoveRows();

    // current index remains the same unless it is equivalent to the new row count

    //    0: 00 Untitled 1
    // -> 1: 01 Untitled 2
    //    2: 02 Untitled 3
    // remove
    //    0: 00 Untitled 1
    // -> 1: 02 Untitled 3
    //
    // mCurrentIndex is still 1

    //    0: 00 Untitled 1
    //    1: 01 Untitled 2
    // -> 2: 02 Untitled 3
    // remove
    //    0: 00 Untitled 1
    // -> 1: 01 Untitled 2
    //
    // mCurrentIndex was 2 and is now 1

    select(rowCount() == row ? row - 1 : row);
    mDocument.setModified(true);
}

void BaseModel::duplicate() {
    int row = nextIndex();
    int rowToDuplicate = mCurrentIndex;

    beginInsertRows(QModelIndex(), row, row);

    mDocument.lock();
    dataDuplicate(rowToDuplicate);
    mDocument.unlock();

    endInsertRows();
    // select the newly duplicated row
    select(row);
    
    mDocument.setModified(true);
}

void BaseModel::rename(const QString &name) {
    dataRename(name);
    mDocument.setModified(true);
    emit dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DisplayRole });
}

void BaseModel::select(int index) {
    if (mCurrentIndex != index) {
        mCurrentIndex = index;
        emit currentIndexChanged(index);
        updateActions();
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
    updateActions();
}

void BaseModel::updateActions() {
    bool hasSelection = mCurrentIndex != -1;
    if (mActNew != nullptr) {
        mActNew->setEnabled(canAdd());
    }

    if (mActRemove != nullptr) {
        mActRemove->setEnabled(hasSelection && canRemove());
    }

    if (mActDuplicate != nullptr) {
        mActDuplicate->setEnabled(hasSelection && canAdd());
    }

    if (mActEdit != nullptr) {
        mActEdit->setEnabled(hasSelection);
    }
}
