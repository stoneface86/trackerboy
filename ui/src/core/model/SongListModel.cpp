
#include "core/model/SongListModel.hpp"


SongListModel::SongMeta::SongMeta(std::string const& name) :
    name(QString::fromStdString(name)),
    shouldCommit(false)
{
}



SongListModel::SongListModel(Module &mod, QObject *parent) :
    QAbstractListModel(parent),
    mModule(mod),
    mSongData()
{
    connect(&mod, &Module::reloaded, this, &SongListModel::reload);
    reload();
}

void SongListModel::commit() {

    int index = 0;
    auto &songs = mModule.data().songs();
    for (auto &meta : mSongData) {
        if (meta.shouldCommit) {
            auto editor = mModule.permanentEdit();
            songs.get(index)->setName(meta.name.toStdString());
            meta.shouldCommit = false;
        }
        ++index;
    }
}

int SongListModel::rowCount(QModelIndex const& index) const {
    return (int)mSongData.size();
}

QVariant SongListModel::data(QModelIndex const& index, int role) const {
    if (role == Qt::DisplayRole) {
        return mSongData[index.row()].name;
    }

    return QVariant();
}

bool SongListModel::setData(QModelIndex const& index, QVariant const& value, int role) {
    if (role == Qt::EditRole) {
        rename(index.row(), value.toString());
        return true;
    }

    return false;
}

void SongListModel::rename(int index, QString const& name) {
    auto &meta = mSongData[index];
    meta.name = name;
    meta.shouldCommit = true;
    auto modelIndex = createIndex(index, 0, nullptr);
    mModule.makeDirty();
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

void SongListModel::append() {
    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    {
        auto editor = mModule.permanentEdit();
        auto &songs = mModule.data().songs();
        songs.append();
        auto song = songs.get(row);
        song->setName("New song");
        mSongData.emplace_back(song->name());
    }

    // add the song to the module, this creates an undo stack for the song
    // (each song has its own command history)
    mModule.addSong();

    endInsertRows();
}

void SongListModel::remove(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    {
        auto editor = mModule.permanentEdit();
        auto &songs = mModule.data().songs();
        songs.remove(index);
        mSongData.erase(mSongData.begin() + index);
    }

    mModule.removeSong(index);

    endRemoveRows();
}

void SongListModel::moveUp(int index) {

    
}


void SongListModel::reload() {

    beginResetModel();

    auto const& songList = mModule.data().songs();
    auto songCount = songList.size();

    mSongData.clear();
    for (int i = 0; i < songCount; ++i) {
        mSongData.emplace_back(songList.get(i)->name());
    }


    endResetModel();

}

