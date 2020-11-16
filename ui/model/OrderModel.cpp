
#include "model/OrderModel.hpp"

#include <algorithm>


OrderModel::OrderModel(ModuleDocument &document, QObject *parent) :
    QAbstractTableModel(parent),
    mDocument(document),
    mOrder(nullptr),
    mCurrentRow(0),
    mCurrentTrack(0)
{
}

void OrderModel::incrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::incdec>(1, selection);
}

void OrderModel::decrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::incdec>(0, selection);
}

void OrderModel::select(int row, int track) {
    
    if (mCurrentRow != row) {
        mCurrentRow = row;
        emit patternChanged(row);
    }
    if (mCurrentTrack != track) {
        mCurrentTrack = track;
        emit trackChanged(track);
    }

    if (mActions.moveUp) {
        mActions.moveUp->setEnabled(mCurrentRow != 0);
    }
    if (mActions.moveDown) {
        mActions.moveDown->setEnabled(mCurrentRow != rowCount() - 1);
    }
}

void OrderModel::setActions(OrderActions actions) {
    mActions = actions;
}

void OrderModel::setOrder(std::vector<trackerboy::Order> *order) {
    beginResetModel();
    mOrder = order;
    endResetModel();
    select(0, 0);
}

void OrderModel::setSelection(QItemSelection const &selection, uint8_t id) {
    modifySelection<ModifyMode::set>(id, selection);
}

// model implementation

int OrderModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4; // 4 columns, 1 for each channel
}

QVariant OrderModel::data(const QModelIndex &index, int role) const {
    if (mOrder != nullptr) {
        if (role == Qt::DisplayRole) {
            auto row = (*mOrder)[index.row()];
            int id = row.tracks[index.column()];
            return QString("%1").arg(id, 2, 16, QLatin1Char('0')).toUpper();
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

Qt::ItemFlags OrderModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    return Qt::ItemIsEditable | 
           Qt::ItemIsSelectable |
           Qt::ItemIsEnabled |
           Qt::ItemNeverHasChildren;
}

QVariant OrderModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            return QString("%1").arg(section, 2, 16, QLatin1Char('0')).toUpper();
        } else {
            return QString("CH%1").arg(section + 1);
        }
    }
    return QVariant();
}

int OrderModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return mOrder == nullptr ? 0 : static_cast<int>(mOrder->size());
}

bool OrderModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {

        bool ok;
        unsigned id = value.toString().toUInt(&ok, 16);
        if (ok && id < 256) {
            mDocument.lock();
            auto &row = (*mOrder)[index.row()];
            row.tracks[index.column()] = static_cast<uint8_t>(id);
            mDocument.unlock();
            return true;
        }
    }

    return false;
}

bool OrderModel::insertRows(int row, int count, QModelIndex const &parent) {
    Q_UNUSED(parent);

    if (mOrder != nullptr) {
        if (rowCount() + count >= 256) {
            return false;
        }
        
        beginInsertRows(parent, row, row + count - 1);
        mOrder->insert(mOrder->cbegin() + row, count, { 0, 0, 0, 0 });
        endInsertRows();
        return true;
    }
    return false;
}

bool OrderModel::removeRows(int row, int count, QModelIndex const &parent) {
    Q_UNUSED(parent);

    if (mOrder != nullptr) {
        if (rowCount() - count < 0) {
            return false;
        }

        beginRemoveRows(parent, row, row + count - 1);
        auto iter = mOrder->cbegin() + row;
        mOrder->erase(iter, iter + count);
        endRemoveRows();
        return true;
    }


    return false;
}



// slots

void OrderModel::insert() {
    _insert({ 0, 0, 0, 0 });
}

void OrderModel::remove() {
    removeRows(mCurrentRow, 1);
    auto rows = rowCount();
    if (rows == 1) {
        mActions.remove->setEnabled(false);
    } else if (rows == 254) {
        mActions.insert->setEnabled(true);
    }
    mDocument.setModified(true);
}

void OrderModel::duplicate() {
    _insert(mOrder->at(mCurrentRow));
}

void OrderModel::moveUp() {
    // moving a row up is just swapping the current row with the previous one
    auto iter = mOrder->begin() + mCurrentRow;
    std::iter_swap(iter, iter - 1);
    emit dataChanged(
        createIndex(mCurrentRow - 1, 0, nullptr),
        createIndex(mCurrentRow, 3, nullptr),
        { Qt::DisplayRole }
    );
    select(mCurrentRow - 1, mCurrentTrack);
}

void OrderModel::moveDown() {
    auto iter = mOrder->begin() + mCurrentRow;
    std::iter_swap(iter, iter + 1);
    emit dataChanged(
        createIndex(mCurrentRow, 0, nullptr),
        createIndex(mCurrentRow + 1, 3, nullptr),
        { Qt::DisplayRole }
    );
    select(mCurrentRow + 1, mCurrentTrack);
}

// private methods

template <OrderModel::ModifyMode mode>
void OrderModel::modifySelection(uint8_t option, QItemSelection const &selection) {
    mDocument.lock();
    for (auto range : selection) {
        for (auto const &index : range.indexes()) {
            auto &id = (*mOrder)[index.row()].tracks[index.column()];
            if constexpr (mode == ModifyMode::incdec) {
                // incdec
                if (option) {
                    if (id != 255) {
                        ++id;
                    }
                } else {
                    if (id != 0) {
                        --id;
                    }
                }
            } else {
                // set
                id = option;
            }

        }
        emit dataChanged(range.topLeft(), range.bottomRight(), { Qt::DisplayRole });
    }
    mDocument.unlock();
}

void OrderModel::_insert(trackerboy::Order order) {
    
    beginInsertRows(QModelIndex(), mCurrentRow, mCurrentRow);
    mOrder->insert(mOrder->cbegin() + mCurrentRow, order);
    endInsertRows();

    auto rows = rowCount();
    if (rows == 2) {
        mActions.remove->setEnabled(true);
    } else if (rows == 255) {
        mActions.insert->setEnabled(false);
    }
    mDocument.setModified(true);
}
