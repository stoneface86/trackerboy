
#include "model/TableModel.hxx"
#include "utils/connectutils.hxx"

static_assert(B::TableCap < 256, "cEmptyId overflow");

TableModel::TableModel(NameListModel *model, QObject *parent)
    : QAbstractListModel(parent)
    , _source(model)
    , _idMap{}
    , _showEmpty(true) {

    load();
    lazyconnect(model, modelReset, this, load);
}

Qt::ItemFlags TableModel::flags(QModelIndex const &index) const {
    return _source->flags(index);
}

int TableModel::rowCount(QModelIndex const &parent) const {
    Q_UNUSED(parent)
    if (_showEmpty) {
        return B::TableCap;
    } else {
        return _source->rowCount();
    }
}

QVariant TableModel::data(QModelIndex const &index, int role) const {
    if (_showEmpty) {
        auto const id = (u8)index.row();
        auto const idInSource = _idMap[id];
        if (idInSource != cEmptyId) {
            return _source->data(createIndex(idInSource, 0), role);
        } else {
            switch (role) {
            case Qt::DisplayRole: {
                auto result = _source->prefixId(id);
                result.append(tr("<empty>"));
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

void TableModel::load() {
    beginResetModel();
    _idMap.fill(cEmptyId);
    u8 listIndex = 0;
    for (auto const &name : _source->list()) {
        _idMap[name.id] = listIndex;
        listIndex++;
    }
    endResetModel();
}
