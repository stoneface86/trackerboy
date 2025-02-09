
#include "model/SongListModel.hpp"

SongListModel::SongMeta::SongMeta(QString const& name) :
    name(name),
    shouldCommit(true)
{
}

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
    connect(&mod, &Module::aboutToSave, this, &SongListModel::commit);
    reload();
}

void SongListModel::commit() {
    // changes to song names are kept in the SongMeta's name variable
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

Qt::ItemFlags SongListModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsSelectable |
               Qt::ItemIsEditable |
               Qt::ItemIsEnabled |
               Qt::ItemNeverHasChildren;
    }

    return Qt::NoItemFlags;
}

int SongListModel::rowCount(QModelIndex const& index) const {
    Q_UNUSED(index)
    return (int)mSongData.size();
}

QVariant SongListModel::data(QModelIndex const& index, int role) const {

    if (index.isValid()) {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole: {
                auto const& meta = mSongData[index.row()];
                if (role == Qt::DisplayRole) {
                    return QStringLiteral("%1# %2").arg(QString::number(index.row() + 1), meta.name);
                } else {
                    return meta.name;
                }
            }
            default:
                break;
        }
    }

    return {};

}

bool SongListModel::setData(QModelIndex const& index, QVariant const& value, int role) {
    if (role == Qt::EditRole) {
        rename(index.row(), value.toString());
        return true;
    }

    return false;
}

void SongListModel::rename(int index, QString const& name) {
    // no need to get an editor from Module: the data being modified is in
    // this class
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
        // add new song meta
        mSongData.emplace_back(mModule.defaultSongName());
    }

    endInsertRows();
}

void SongListModel::remove(int index) {
    trackerboy::Song *removedSong;
    beginRemoveRows(QModelIndex(), index, index);
    {
        auto editor = mModule.permanentEdit();
        auto &songs = mModule.data().songs();
        removedSong = songs.get(index);
        songs.remove(index);
        mSongData.erase(mSongData.begin() + index);
    }

    endRemoveRows();

    // now remove the history for the song (if exists)
    mModule.removeHistory(removedSong);
}

void SongListModel::duplicate(int index) {
    beginInsertRows(QModelIndex(), index, index);
    {
        auto editor = mModule.permanentEdit();
        auto &songs = mModule.data().songs();
        songs.duplicate(index);
        mSongData.emplace(mSongData.begin() + index + 1, mSongData[index]);
    }

    endInsertRows();
}

void SongListModel::moveUp(int index) {
    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index - 1);
    {
        auto editor = mModule.permanentEdit();
        mModule.data().songs().moveUp(index);
    }

    auto iter = mSongData.begin() + index;
    std::iter_swap(iter, iter - 1);

    endMoveRows();

    
}

void SongListModel::moveDown(int index) {
    // the + 2 is correct, qt is weird
    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index + 2);
    {
        auto editor = mModule.permanentEdit();
        mModule.data().songs().moveDown(index);
    }

    auto iter = mSongData.begin() + index;
    std::iter_swap(iter, iter + 1);

    endMoveRows();
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

