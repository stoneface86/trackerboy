
#include "model/SongListModel.hxx"
#include "utils/connectutils.hxx"

SongListModel::SongListModel(Document const *doc, QObject *parent)
    : QAbstractListModel(parent)
    , mDocument(doc) {
    lazyconnect(doc, reloaded, this, reload);
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
    return (int)mDocument->source()->songCount();
}

QVariant SongListModel::data(QModelIndex const &index, int role) const {

    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole: {
            auto const songNo = index.row();
            QString result = QString::number(songNo + 1);
            result.append(". ");
            auto const name = mDocument->source()->songName(songNo);
            result.append(name.data);
            return result;
        }
        default:
            break;
        }
    }

    return {};
}

void SongListModel::reload() {
    beginResetModel();
    endResetModel();
}
