
#include "model/BaseModel.hpp"




BaseModel::BaseModel(ModuleDocument &document) :
    QAbstractListModel(),
    mDocument(document),
    mCurrentIndex(-1),
    mActNew(nullptr),
    mActRemove(nullptr),
    mActDuplicate(nullptr),
    mActEdit(nullptr),
    mCanSelect(true)
{
}

int BaseModel::currentIndex() const {
    return mCurrentIndex;
}

void BaseModel::add() {
    int row = nextIndex();

    mCanSelect = false;
    beginInsertRows(QModelIndex(), row, row);

    {
        auto ctx = mDocument.beginEdit();
        dataAdd();
    }
    endInsertRows();
    mCanSelect = true;

    // select the newly inserted row
    select(row);
}

QString BaseModel::name() {
    return nameAt(mCurrentIndex);
}

void BaseModel::remove() {
    remove(mCurrentIndex);
}

void BaseModel::remove(int index) {
    
    mCanSelect = false;
    beginRemoveRows(QModelIndex(), index, index);

    {
        auto ctx = mDocument.beginEdit();
        dataRemove(index);
    }

    endRemoveRows();
    mCanSelect = true;

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

    if (index == mCurrentIndex && index == rowCount()) {
        select(index - 1);
    }
}

void BaseModel::duplicate() {
    duplicate(mCurrentIndex);
}

void BaseModel::duplicate(int index) {
    int row = nextIndex();

    mCanSelect = false;
    beginInsertRows(QModelIndex(), row, row);

    {
        auto ctx = mDocument.beginEdit();
        dataDuplicate(index);
    }
    endInsertRows();
    mCanSelect = true;

    // select the newly duplicated row
    select(row);
    
}

void BaseModel::rename(const QString &name) {
    rename(mCurrentIndex, name);
}

void BaseModel::rename(int index, const QString &name) {
    {
        auto ctx = mDocument.beginEdit();
        dataRename(index, name);
    }
    
    emit dataChanged(createIndex(mCurrentIndex, 0, nullptr), createIndex(mCurrentIndex, 0, nullptr), { Qt::DisplayRole });
}

void BaseModel::select(int index) {
    if (mCanSelect && mCurrentIndex != index) {
        mCurrentIndex = index;
        dataSelected(index);
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

void BaseModel::dataSelected(int index) {
    (void)index; // do nothing
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
