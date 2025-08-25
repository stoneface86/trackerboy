
#include "model/SongListEditorModel.hxx"
#include "utils/backendutils.hxx"

#include <QMimeData>

SongListEditorModel::SongListEditorModel(NameListModel *sourceModel,
                                         QObject *parent)
    : QAbstractItemModel(parent)
    , _source(sourceModel)
    , _items() {
    setListFromSource();
}

Qt::ItemFlags SongListEditorModel::flags(QModelIndex const &index) const {
    Qt::ItemFlags result;
    if (index.isValid()) {
        constexpr auto defaultFlags = Qt::ItemIsSelectable |
                                      Qt::ItemIsDragEnabled |
                                      Qt::ItemIsDropEnabled;
        constexpr auto enDefault = defaultFlags | Qt::ItemIsEnabled;
        switch (index.column()) {
        case colName:
            result = enDefault | Qt::ItemIsEditable;
            break;
        case colRemove:
            if (_items.size() > 1) {
                result = enDefault | Qt::ItemIsUserCheckable;
            } else {
                // song list must have at least 1 song, disable the checkbox
                result = defaultFlags | Qt::ItemIsUserCheckable;
            }
            break;
        default:
            result = enDefault;
        }
    }

    return result;
}

QModelIndex SongListEditorModel::index(int row, int column,
                                       QModelIndex const &parent) const {
    if (parent.isValid()) {
        return {};
    }
    return createIndex(row, column);
}

QModelIndex SongListEditorModel::parent(QModelIndex const &index) const {
    Q_UNUSED(index)
    return {};
}

int SongListEditorModel::columnCount(QModelIndex const &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return colCount;
    }
}

int SongListEditorModel::rowCount(QModelIndex const &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return _items.size();
    }
}

QVariant SongListEditorModel::data(QModelIndex const &index, int role) const {
    if (index.isValid()) {
        auto const row = index.row();
        switch (index.column()) {
        case colNumber:
            if (role == Qt::DisplayRole) {
                return QString::number(row + 1);
            }
            break;
        case colName:
            if (role == Qt::DisplayRole || role == Qt::EditRole) {
                return _items[row].name;
            }
            break;
        case colRemove:
            if (role == Qt::CheckStateRole) {
                return (_items[row].action == itemRemove) ? Qt::Checked
                                                          : Qt::Unchecked;
            }
            break;
        case colStatus:
            if (role == Qt::DisplayRole) {
                auto const &item = _items[row];
                switch (item.action) {
                case itemKeep:
                    if (item.nameModified) {
                        return tr("Renamed");
                    } else {
                        return tr("Unchanged");
                    }
                case itemNew:
                    return tr("Add new");
                case itemDuplicate:
                    return tr("Duplicate of #%1").arg(1 + (int)item.sourceId);
                case itemRemove:
                    return tr("Pending removal");
                }
            }
            break;
        }
    }
    return {};
}

bool SongListEditorModel::setData(QModelIndex const &index,
                                  QVariant const &value, int role) {
    if (index.isValid()) {
        auto const row = index.row();
        switch (index.column()) {
        case colNumber:
            return false;
        case colName:
            if (role == Qt::EditRole) {
                auto &item = _items[row];
                auto const newName = value.toString();
                if (item.name != newName) {
                    item.nameModified = true;
                    item.name = newName;
                }
                return true;
            }
            break;
        case colRemove:
            if (role == Qt::CheckStateRole) {
                auto &item = _items[row];
                auto const prevaction = item.action;
                if (value.toInt() == Qt::Checked) {
                    if (item.action == itemNew ||
                        item.action == itemDuplicate) {
                        // remove from the list
                        beginRemoveRows(QModelIndex(), row, row);
                        _items.removeAt(row);
                        endRemoveRows();
                    } else {
                        if (_items.size() - countRemoved() > 1) {
                            item.action = itemRemove;
                        } else {
                            // user is trying to remove all songs:
                            // list must have at least one
                            return false;
                        }
                    }

                } else {
                    item.action = itemKeep;
                }
                if (item.action != prevaction) {
                    // status column changes when changing action
                    auto const index = createIndex(row, colStatus);
                    emit dataChanged(index, index);
                }
                return true;
            }
            break;
        }
    }
    return false;
}

Qt::DropActions SongListEditorModel::supportedDropActions() const {
    return Qt::MoveAction;
}

QMimeData *SongListEditorModel::mimeData(QModelIndexList const &indexes) const {
    QMimeData *result = nullptr;
    if (indexes.size() > 0) {
        result = new QMimeData;
        result->setText(QString::number(indexes[0].row()));
    }
    return result;
}

QStringList SongListEditorModel::mimeTypes() const {
    QStringList result;
    result.append("text/plain");
    return result;
}

bool SongListEditorModel::dropMimeData(QMimeData const *data,
                                       Qt::DropAction action, int row,
                                       int column, QModelIndex const &parent) {
    Q_UNUSED(column)
    if (action == Qt::MoveAction && data && data->hasText()) {
        bool ok = false;
        auto const srcRow = data->text().toInt(&ok);
        if (ok) {
            // determine where to move it to
            int destRow;
            if (row == -1) {
                destRow = parent.row();
            } else {
                destRow = row;
            }
            _items.move(srcRow, destRow);
            emit dataChanged(createIndex(qMin(srcRow, destRow), 0),
                             createIndex(qMax(srcRow, destRow), colStatus));
            return true;
        }
    }
    return false;
}

QVariant SongListEditorModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case colNumber:
            return tr("Number");
        case colName:
            return tr("Name");
        case colRemove:
            return tr("Remove?");
        case colStatus:
            return tr("Pending result");
        }
    }
    return {};
}

void SongListEditorModel::add() {
    auto const at = _items.size();
    beginInsertRows(QModelIndex(), at, at);
    _items.append({-1, itemNew, true, _source->document()->defaultSongName()});
    endInsertRows();
}

void SongListEditorModel::duplicate(int index) {
    Q_ASSERT(index >= 0 && index < _items.size());
    auto const at = index + 1;
    beginInsertRows({}, at, at);
    auto dup = _items[index];
    dup.action = itemDuplicate;
    dup.nameModified = true;
    dup.name = tr("Copy of %1").arg(dup.name);
    _items.append(dup);
    endInsertRows();
}

void SongListEditorModel::moveUp(int index) {
    if (index > 0) {
        moveDown(index - 1);
    }
}

void SongListEditorModel::moveDown(int index) {
    auto const neighbor = index + 1;
    if (neighbor < _items.size()) {
        _items.swapItemsAt(index, neighbor);
        emit dataChanged(createIndex(index, colNumber),
                         createIndex(neighbor, colStatus));
    }
}

void SongListEditorModel::reset() {
    beginResetModel();
    setListFromSource();
    endResetModel();
}

void SongListEditorModel::apply(Document &doc) {
    // see backend/data.nim
    auto changes = B::initSongListChanges();
    NameList newNames;
    for (auto const &item : _items) {
        Name name;
        switch (item.action) {
        case itemKeep:
            changes.keepOriginal(item.sourceId);
            name.changed =
                _source->list()[item.sourceId].changed || item.nameModified;
            break;
        case itemNew:
            changes.addNew();
            name.changed = true;
            break;
        case itemDuplicate:
            changes.duplicate(item.sourceId);
            name.changed = true;
            break;
        case itemRemove:
            continue; // don't add the name to newNames
        }
        name.id = (i8)newNames.size();
        name.value = item.name;
        newNames.append(std::move(name));
    }

    beginResetModel();

    doc.changeSongList(changes);
    _source->setList(newNames);
    setListFromSource();

    endResetModel();
}

void SongListEditorModel::setListFromSource() {
    _items.resize(_source->rowCount());
    int index = 0;
    for (auto &item : _items) {
        item.sourceId = (qint8)index;
        item.action = itemKeep;
        item.nameModified = false;
        item.name = _source->name(index);
        ++index;
    }
}

int SongListEditorModel::countRemoved() const {
    int result = 0;
    for (auto const &item : _items) {
        if (item.action == itemRemove) {
            ++result;
        }
    }
    return result;
}