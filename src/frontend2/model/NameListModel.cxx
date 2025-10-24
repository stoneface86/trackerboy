
#include "model/NameListModel.hxx"
#include "utils/backendutils.hxx"
#include "utils/connectutils.hxx"
#include "utils/string.hxx"

NameListModel::NameListModel(Document *doc, BItemCategory const cat,
                             QString defaultName, QObject *parent)
    : QAbstractListModel(parent)
    , _document(doc)
    , _list()
    , _cat(cat)
    , _itemizer(cat)
    , _defaultName(std::move(defaultName)) {

    lazyconnect(_document, reloaded, this, load);
    lazyconnect(_document, aboutToSave, this, commit);

    load(true);
}

Document *NameListModel::document() const {
    return _document;
}

BItemCategory NameListModel::category() const {
    return _cat;
}

QString const &NameListModel::defaultName() const {
    return _defaultName;
}

Qt::ItemFlags NameListModel::flags(QModelIndex const &index) const {
    if (index.isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
               Qt::ItemNeverHasChildren;
    }

    return Qt::NoItemFlags;
}

int NameListModel::rowCount(QModelIndex const &index) const {
    Q_UNUSED(index)
    return (int)_list.size();
}

QVariant NameListModel::data(QModelIndex const &index, int const role) const {
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            auto const &name = _list[index.row()];
            QString result = prefixId(name.id);
            result.append(name.value);
#ifdef QT_DEBUG
            // add a '*' for changed names for debugging purposes
            if (name.changed) {
                result.append('*');
            }
#endif
            return result;
        }
    }

    return {};
}

QString NameListModel::prefixId(u8 const id) const {
    QString result;
    if (_cat == B::catSong) {
        result = QString::number((int)id + 1);
        result.append(". ");
    } else {
        // catInstrument, catWaveform
        result = toHex(id);
        result.append(" - ");
    }
    return result;
}

QString const &NameListModel::name(int const index) const {
    return _list[index].value;
}

void NameListModel::setName(int const index, QString const &name) {
    if (auto &at = _list[index]; at.value != name) {
        at.value = name;
        at.changed = true;
        auto const mindex = createIndex(index, 0);
        emit dataChanged(mindex, mindex);
    }
}

NameList const &NameListModel::list() const {
    return _list;
}

void NameListModel::setList(NameList const &list) {
    beginResetModel();
    _list = list;
    endResetModel();
}

void NameListModel::load(bool const newModule) {
    beginResetModel();
    if (newModule) {
        _list.clear();
        if (_cat == B::catSong) {
            _list.append({true, 0, _defaultName});
        }
    } else {
        auto const view = _document->view();
        auto const size = _itemizer.count(view->mod);
        u8 lastId = 0;
        _list.resize(size);
        for (NI i = 0; i < size; ++i) {
            auto &item = _list[i];
            item.changed = false;
            auto const name = _itemizer.name(view->mod, lastId);
            item.id = name.id;
            item.value = toQString(name.value);
            lastId = name.id + 1;
        }
    }
    endResetModel();
}

void NameListModel::commit() {
    auto edit = _document->edit();
    for (auto &name : _list) {
        if (name.changed) {
            name.changed = false;
            _itemizer.setName(edit->mod, name.id, toNimString(name.value).s);
#ifdef QT_DEBUG
            emit dataChanged(createIndex(name.id, 0), createIndex(name.id, 0));
#endif
        }
    }
}

void NameListModel::insert(int const at, u8 const id, QString const &name) {
    beginInsertRows({}, at, at);
    _document->setModified();
    _list.insert(at, {true, id, name});
    endInsertRows();
}

u8 NameListModel::remove(int const at) {
    beginRemoveRows({}, at, at);
    _document->setModified();
    auto const result = _list[at].id;
    _list.remove(at);
    endRemoveRows();
    return result;
}