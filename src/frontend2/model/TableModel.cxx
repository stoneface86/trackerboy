
#include "model/TableModel.hxx"
#include "utils/connectutils.hxx"

#include <utility>

#include <QFont>

TableModel::TableModel(NameListModel *model, QObject *parent)
    : QAbstractListModel(parent)
    , _source(model)
    , _backend() {
    B::initTableModel(model->category(), &_backend);
    load();
    lazyconnect(model, modelReset, this, load);
}

Qt::ItemFlags TableModel::flags(QModelIndex const &index) const {
    return _source->flags(index);
}

int TableModel::rowCount(QModelIndex const &parent) const {
    Q_UNUSED(parent)
    if (_backend.showEmpty) {
        return B::TableCap;
    } else {
        return _source->rowCount();
    }
}

QVariant TableModel::data(QModelIndex const &index, int const role) const {
    if (_backend.showEmpty) {
        auto const id = (u8)index.row();
        if (auto const listIndex = _backend.listIndex(id); listIndex != -1) {
            return _source->data(createIndex(listIndex, 0), role);
        } else {
            switch (role) {
            case Qt::DisplayRole:
                return _source->prefixId(id);
            case Qt::FontRole: {
                QFont result;
                result.setItalic(true);
                return result;
            }
            default:
                return {};
            }
        }
    } else {
        return _source->data(index, role);
    }
}

void TableModel::add(int const at) {
    auto edit = _source->document()->edit(true);
    itemAdded(_backend.add(edit->mod, at), _source->defaultName());
}

void TableModel::remove(int const at) {
    auto edit = _source->document()->edit(true);
    itemRemoved(at);
    _backend.remove(edit->mod, at);
}

void TableModel::duplicate(int const at) {
    auto edit = _source->document()->edit(true);
    itemAdded(_backend.duplicate(edit->mod, at),
              _source->name(_backend.listIndex(at)));
}

bool TableModel::canAdd() const {
    return _source->rowCount() < B::TableCap;
}

bool TableModel::hasId(int const id) const {
    return id >= 0 && id <= B::TableCap && _backend.listIndex(id) != -1;
}

int TableModel::id(QModelIndex const &index) const {
    if (!index.isValid()) {
        return -1;
    }
    if (_backend.showEmpty) {
        return index.row();
    } else {
        return _source->list()[index.row()].id;
    }
}

QModelIndex TableModel::indexFromTable(int const tableId) const {
    if (tableId >= 0 && tableId < B::TableCap) {
        if (_backend.showEmpty) {
            return createIndex(tableId, 0);
        } else {
            return createIndex(_backend.listIndex(tableId), 0);
        }
    }
    return {};
}

void TableModel::setShowEmpty(const bool show) {
    if (show != _backend.showEmpty) {
        beginResetModel();
        _backend.showEmpty = show;
        endResetModel();
    }
}

void TableModel::load() {
    beginResetModel();
    _backend.reset();
    u8 listIndex = 0;
    for (auto const &name : _source->list()) {
        _backend.assignId(name.id, listIndex);
        listIndex++;
    }
    endResetModel();
}

void TableModel::itemChanged(u8 const id) {
    auto const index = createIndex(id, 0);
    dataChanged(index, index);
}

void TableModel::itemAdded(u8 const id, QString const &name) {
    auto const listIndex = _backend.listIndex(id);
    _source->insert(listIndex, id, name);
    if (_backend.showEmpty) {
        itemChanged(id);
    } else {
        beginInsertRows({}, listIndex, listIndex);
        endInsertRows();
    }
}

void TableModel::itemRemoved(u8 const id) {
    auto const listIndex = _backend.listIndex(id);
    if (_backend.showEmpty) {
        itemChanged(id);
    } else {
        beginRemoveRows({}, listIndex, listIndex);
        endRemoveRows();
    }
    _source->remove(listIndex);
}
