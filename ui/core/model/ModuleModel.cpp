
#include "core/model/ModuleModel.hpp"

#include <QtDebug>
#include <QElapsedTimer>

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
    mDocuments(),
    mCurrent(nullptr)
{
}


Qt::ItemFlags ModuleModel::flags(QModelIndex const& index) const {
    if (index.isValid()) {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;

        ModelId id;
        id = index.internalId();
        
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
        ModelId id;
        id = index.internalId();

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
        ModelId id;
        id = index.internalId();
        
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

        ModelId parentId;
        parentId = parent.internalId();
        
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

        ModelId id;
        id = index.internalId();
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
        ModelId parentId; parentId.data = parent.internalId();

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
            default:
                // child data from models do not have children
                return 0;

        }
    } else {
        return mDocuments.size();
    }
}

int ModuleModel::columnCount(QModelIndex const& parent) const {
    Q_UNUSED(parent)
    // always 1 column
    return 1;
}

QModelIndex ModuleModel::addDocument(ModuleDocument *doc) {
    int row = mDocuments.size();
    beginInsertRows(QModelIndex(), row, row);

    mDocuments.push_back(doc);
    mUndoGroup.addStack(&doc->undoStack());

    endInsertRows();

    connectChildModel(doc->orderModel());
    connectChildModel(doc->instrumentModel());
    connectChildModel(doc->waveModel());

    return createIndex(row, 0, ModelId().data);
}

void ModuleModel::removeDocument(ModuleDocument *doc) {
    auto row = mDocuments.indexOf(doc);
    
    if (row != -1) {

        beginRemoveRows(QModelIndex(), row, row);
        mDocuments.remove(row);
        mUndoGroup.removeStack(&doc->undoStack());
        endRemoveRows();

        doc->orderModel().disconnect(this);
        doc->instrumentModel().disconnect(this);
        doc->waveModel().disconnect(this);
    }
}

QVector<ModuleDocument*> const& ModuleModel::documents() const noexcept {
    return mDocuments;
}

void ModuleModel::setCurrentDocument(int index) {
    ModuleDocument *doc;
    if (index == -1) {
        doc = nullptr;
    } else {
        doc = mDocuments[index];
    }

    if (mCurrent != doc) {
        mCurrent = doc;
        if (doc) {
            mUndoGroup.setActiveStack(&doc->undoStack());
        }
        // emitting this signal triggers a "document-switch"
        // benchmark this for usability reasons
        // if it's too high then the user experience will suffer, switching
        // tabs should be instantaneous
        #ifndef QT_NO_DEBUG_OUTPUT
        QElapsedTimer timer;
        timer.start();
        #endif
        
        emit currentDocumentChanged(doc);
        
        #ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "Document switched in" << timer.elapsed() << "milliseconds";
        #endif
    }
}

ModuleDocument* ModuleModel::currentDocument() const noexcept {
    return mCurrent;
}

int ModuleModel::documentIndex(QModelIndex const& index) {
    if (index.isValid()) {
        ModelId id; id.data = index.internalId();
        if (id.level() == 0) {
            return index.row();
        } else {
            return id.documentIndex();
        }
    } else {
        return -1;
    }
}

ModuleModel::ItemType ModuleModel::itemAt(QModelIndex const& index) {
    if (index.isValid()) {

        ModelId id; id = index.internalId();

        switch (id.level()) {
            case 0:
                return ItemType::document;
            case 1:
                switch (index.row()) {
                    case 0:
                        return ItemType::instruments;
                    case 1:
                        return ItemType::orders;
                    case 2:
                        return ItemType::waveforms;
                    case 3:
                        return ItemType::settings;
                    default:
                        break;
                }
            case 2:
                switch (id.parent()) {
                    case 0:
                        return ItemType::instrument;
                    case 1:
                        return ItemType::order;
                    case 2:
                        return ItemType::waveform;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

    }

    return ItemType::invalid;
}

QUndoGroup& ModuleModel::undoGroup() noexcept {
    return mUndoGroup;
}

void ModuleModel::moveDocument(int from, int to) {
    beginMoveRows(
        QModelIndex(),
        from,
        from,
        QModelIndex(),
        from < to ? to + 1 : to
    );
    mDocuments.move(from, to);
    endMoveRows();
}

// the following methods forward any row insertions/removals from child models
// to this model. jank level: mild

template <class T>
void ModuleModel::connectChildModel(T &model) {
    connect(&model, &T::rowsInserted, this, &ModuleModel::insertChildRows<T>);
    // we use the aboutToBeRemoved signal so that the rowCount from the child
    // model has not been modified yet (if it was then an assert in Qt's source
    // will fail)
    connect(&model, &T::rowsAboutToBeRemoved, this, &ModuleModel::removeChildRows<T>);
    connect(&model, &T::dataChanged, this, &ModuleModel::childDataChanged<T>);
}

template <class T>
void ModuleModel::insertChildRows(QModelIndex const& index, int first, int last) {
    Q_UNUSED(index)
    _insertChildRows(getChildNode<T>(sender()), first, last);
}

void ModuleModel::_insertChildRows(ChildModelContext ctx, int first, int last) {
    ModelId id((unsigned)mDocuments.indexOf(std::get<0>(ctx)));
    beginInsertRows(createIndex(std::get<1>(ctx), 0, id.data), first, last);
    endInsertRows();
}

template <class T>
void ModuleModel::removeChildRows(QModelIndex const& index, int first, int last) {
    Q_UNUSED(index)
    _removeChildRows(getChildNode<T>(sender()), first, last);
}

void ModuleModel::_removeChildRows(ChildModelContext ctx, int first, int last) {
    ModelId id((unsigned)mDocuments.indexOf(std::get<0>(ctx)));
    beginRemoveRows(createIndex(std::get<1>(ctx), 0, id.data), first, last);
    endRemoveRows();
}

template <class T>
void ModuleModel::childDataChanged(
    const QModelIndex &topLeft,
    const QModelIndex &bottomRight,
    const QVector<int> &roles) 
{
    _childDataChanged(getChildNode<T>(sender()), topLeft, bottomRight, roles);
}

void ModuleModel::_childDataChanged(
    ChildModelContext ctx,
    const QModelIndex &topLeft,
    const QModelIndex &bottomRight,
    const QVector<int> &roles) 
{
    ModelId id((unsigned)mDocuments.indexOf(std::get<0>(ctx)), std::get<1>(ctx));
    emit dataChanged(
        createIndex(topLeft.row(), 0, id.data),
        createIndex(bottomRight.row(), 0, id.data),
        roles
    );
}



template <>
ModuleModel::ChildModelContext ModuleModel::getChildNode<InstrumentListModel>(QObject *sender) {
    return {
        &(static_cast<InstrumentListModel*>(sender)->document()),
        0
    };
}

template <>
ModuleModel::ChildModelContext ModuleModel::getChildNode<OrderModel>(QObject *sender) {
    return {
        &(static_cast<OrderModel*>(sender)->document()),
        1
    };
}

template <>
ModuleModel::ChildModelContext ModuleModel::getChildNode<WaveListModel>(QObject *sender) {
    return {
        &(static_cast<WaveListModel*>(sender)->document()),
        2
    };
}
