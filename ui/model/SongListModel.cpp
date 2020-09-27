
#include "model/SongListModel.hpp"

SongListModel::SongListModel(ModuleDocument &document) :
    mSongVector(document.songs()),
    BaseModel(document)
{
}

int SongListModel::rowCount(const QModelIndex &parent) const {
    return mSongVector.size();
}

QVariant SongListModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        QString fmt("%1: %2");
        return QVariant(fmt.arg(
            QString::number(index.row() + 1),
            QString::fromStdString(mSongVector[index.row()].name())
        ));
    }
    return QVariant();
}


QString SongListModel::name() {
    return QString::fromStdString(mSongVector[mCurrentIndex].name());
}

bool SongListModel::canAdd() {
    return mSongVector.size() < 256;
}

bool SongListModel::canRemove() {
    // must have at least one song
    return mSongVector.size() > 1;
}

void SongListModel::dataAdd() {
    mSongVector.emplace_back();
}

void SongListModel::dataRemove(int row) {
    mSongVector.erase(mSongVector.begin() + row);
}

void SongListModel::dataDuplicate(int row) {
    // emplace using Song's copy constructor
    mSongVector.emplace_back(mSongVector[row]);
}

void SongListModel::dataRename(const QString &name) {
    mSongVector[mCurrentIndex].setName(name.toStdString());
}

int SongListModel::nextIndex() {
    // songs always get added/duplicated to the end of the list
    return mSongVector.size();
}
