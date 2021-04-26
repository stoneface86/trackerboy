
#include "core/model/ModuleModel.hpp"

#include <QtDebug>

#include <array>
#include <type_traits>

static std::array const MODULE_NODE_NAMES = {
    QT_TR_NOOP("Instruments"),
    QT_TR_NOOP("Song order"),
    QT_TR_NOOP("Waveforms"),
    QT_TR_NOOP("Settings")
};

//
// all QModelIndexes created by this model carry a ModelId, stored in its internalId property.
// This id determines the Index's location in the tree. The tree has 3 levels.
// 
// Ex:
// Module                           <- level 0
//  * Instruments                   <- level 1
//    * 00 - Untitled 00            <- level 2
//  * Song order                    
//    * Pattern 00: 00 00 00 00     
//  * Waveforms                     
//    * 00 - triangle               
//    * 01 - square
//
struct ModelId {
    static_assert(sizeof(quintptr) >= 3, "cannot use quintptr to store ModelId!");
    
    // level 0
    constexpr ModelId() :
        data(0u)
    {
    }

    // level 1
    constexpr explicit ModelId(unsigned documentIndex) :
        data(documentIndex << 16 | (1))
    {
    }

    // level 2
    constexpr ModelId(unsigned documentIndex, unsigned parent) :
        data((documentIndex << 16) | (parent << 8) | 2)
    {
    }

    constexpr ModelId(quintptr data) :
        data(data)
    {
    }

    constexpr ModelId& operator=(quintptr data_) {
        data = data_;
        return *this;
    }

    constexpr unsigned level() const noexcept {
        return data & 0xFF;
    }

    constexpr unsigned parent() const noexcept {
        return (data >> 8) & 0xFF;
    }

    constexpr unsigned documentIndex() const noexcept {
        return (data >> 16) & 0xFF;
    }

    quintptr data;
};






ModuleModel::ModuleModel(QObject *parent) :
    QAbstractItemModel(parent),
    mDocuments()
{
}


Qt::ItemFlags ModuleModel::flags(QModelIndex const& index) const {
    if (index.isValid()) {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;

        ModelId id = index.internalId();
        if (id.level() == 2) {
            flags |= Qt::ItemIsSelectable;
        }

        return flags;
    } else {
        return Qt::NoItemFlags;
    }
}

QVariant ModuleModel::data(QModelIndex const& index, int role) const {
    if (index.isValid()) {
        ModelId id = index.internalId();

        switch (id.level()) {
            case 0:
                if (role == Qt::DisplayRole) {
                    return mDocuments[index.row()]->name();
                }
                break;
            case 1: 
                if (role == Qt::DisplayRole) {
                    auto row = index.row();
                    Q_ASSERT(row < MODULE_NODE_NAMES.size());
                    return tr(MODULE_NODE_NAMES[row]);
                }
                break;
            case 2: {
                auto doc = mDocuments[id.documentIndex()];
                switch (id.parent()) {
                    case 0: {
                        auto &model = doc->instrumentModel();
                        return model.data(model.index(index.row()), role);
                    }
                    case 1:
                        if (role == Qt::DisplayRole) {
                            auto row = index.row();
                            return tr("Pattern %1")
                                .arg(row, 2, 16, QChar('0'));

                        }
                        break;
                    case 2: {
                        auto &model = doc->waveModel();
                        return model.data(model.index(index.row()), role);
                    }
                    default:
                        break;
                }

                break;
            }
            default:
                break;
        }
    }

    return QVariant();
}

bool ModuleModel::hasChildren(QModelIndex const& index) const {
    // only modules and table nodes have children
    if (index.isValid()) {
        ModelId id = index.internalId();
        switch (id.level()) {
            case 0:
                return true;
            case 1:
                return index.row() != 3;
            default:
                return false;
        }
    }

    return true;
}

QVariant ModuleModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    // no header data
    return QVariant();
}

QModelIndex ModuleModel::index(int row, int column, QModelIndex const& parent) const {

    if (row < 0 || row >= rowCount(parent) || column != 0) {
        return QModelIndex();
    }

    ModelId id;

    if (parent.isValid()) {

        ModelId parentId = parent.internalId();
        switch (parentId.level()) {
            case 0:
                id = ModelId((unsigned)parent.row());
                break;
            case 1:
                id = ModelId(parentId.documentIndex(), (unsigned)parent.row());
                break;
            default:
                // this should never happen (level 2 indices have no children, the ModelId is faulty)
                return QModelIndex();
        }


    }

    return createIndex(row, 0, id.data);

}

QModelIndex ModuleModel::parent(QModelIndex const& index) const {
    if (index.isValid()) {

        ModelId id = index.internalId();
        switch (id.level()) {
            case 0:
                break; // return invalid to serve as the root
            case 1:
                return createIndex(id.documentIndex(), 0, ModelId().data);
            case 2:
                return createIndex(id.parent(), 0, ModelId(id.documentIndex()).data);
            default:
                // should never happen (we only have 3 levels)
                break;
        }
    }

    return {};
}

int ModuleModel::rowCount(QModelIndex const& parent) const {
    if (parent.isValid()) {
        ModelId parentId = parent.internalId();

        switch (parentId.level()) {
            case 0:
                // every module has 4 items (instruments, order, waveforms and setings)
                return 4;
            case 1: {
                auto doc = mDocuments[parentId.documentIndex()];
                switch (parent.row()) {
                    case 0: // instruments
                        return doc->instrumentModel().rowCount();
                    case 1:
                        return doc->orderModel().rowCount();
                    case 2:
                        return doc->waveModel().rowCount();
                    default:
                        return 0;
                }
            }
                // TODO: get rowCount from associated models
                /*switch (parent.row()) {

                }
                break;*/
            case 2:
                // child data from models do not have children
                return 0;

        }
    } else {
        return mDocuments.size();
    }
}

int ModuleModel::columnCount(QModelIndex const& parent) const {
    // always 1 column
    return 1;
}

QModelIndex ModuleModel::addDocument(ModuleDocument *doc) {
    int row = mDocuments.size();
    beginInsertRows(QModelIndex(), row, row);

    mDocuments.push_back(doc);

    endInsertRows();

    return createIndex(row, 0, ModelId().data);
}

void ModuleModel::removeDocument(ModuleDocument *doc) {
    auto row = mDocuments.indexOf(doc);
    
    if (row != -1) {

        beginRemoveRows(QModelIndex(), row, row);
        mDocuments.remove(row);
        endRemoveRows();
    }
}
