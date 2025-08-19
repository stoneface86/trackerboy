
#include "model/SongListModel.hxx"
#include "utils/backendutils.hxx"
#include "utils/connectutils.hxx"

SongListModel::SongListModel(Document *doc, QObject *parent)
    : QAbstractListModel(parent)
    , _document(doc)
    , _nameCache() {
    lazyconnect(doc, reloaded, this, reload);
    lazyconnect(doc, aboutToSave, this, commit);
    reload(true);
}

Qt::ItemFlags SongListModel::flags(QModelIndex const &index) const {
    if (index.isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
               Qt::ItemNeverHasChildren;
    }

    return Qt::NoItemFlags;
}

int SongListModel::rowCount(QModelIndex const &index) const {
    Q_UNUSED(index)
    return _nameCache.size();
}

QVariant SongListModel::data(QModelIndex const &index, int role) const {

    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole: {
            auto const songNo = index.row();
            QString result = QString::number(songNo + 1);
            result.append(". ");
            result.append(_nameCache[songNo]);
            return result;
        }
        default:
            break;
        }
    }

    return {};
}

QString SongListModel::name(int song) const {
    return _nameCache[song];
}

void SongListModel::setName(int song, QString const &name) {
    _document->setModified();
    _nameCache[song] = name;
    _cacheDirty = true;
    dataChanged(createIndex(song, 0), createIndex(song, 0));
}

void SongListModel::reload(bool newModule) {
    beginResetModel();
    {
        _nameCache.clear();
        if (newModule) {
            _nameCache.append(_document->defaultSongName());
        } else {
            auto view = _document->view();
            auto const songCount = view->mod.songCount();
            _cacheDirty = false;
            for (B::NI i = 0; i < songCount; ++i) {
                _nameCache.append(toQString(view->mod.songName(i)));
            }
        }
    }
    endResetModel();
}

void SongListModel::setNames(QStringList const &names) {
    beginResetModel();
    _nameCache = names;
    _cacheDirty = true;
    endResetModel();
}

void SongListModel::commit() {
    if (_cacheDirty) {
        auto edit = _document->edit();
        B::NI songNo = 0;
        for (auto name : _nameCache) {
            edit->mod.setSongName(songNo, toNimString(_nameCache[songNo]).s);
            ++songNo;
        }
        _cacheDirty = false;
    }
}
