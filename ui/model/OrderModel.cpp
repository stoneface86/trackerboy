
#include "model/OrderModel.hpp"


OrderModel::OrderModel(ModuleDocument &document, QObject *parent) :
    QAbstractTableModel(parent),
    mDocument(document),
    mOrder(nullptr),
    mCurrentRow(0),
    mCurrentTrack(0)
{
}

void OrderModel::setActions(OrderActions actions) {
    mActions = actions;
}

int OrderModel::columnCount(const QModelIndex &parent) const {
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
    return mOrder == nullptr ? 0 : mOrder->size();
}

bool OrderModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {

        bool ok;
        unsigned id = value.toString().toUInt(&ok, 16);
        if (ok) {
            mDocument.lock();
            auto &row = (*mOrder)[index.row()];
            row.tracks[index.column()] = id;
            mDocument.unlock();
            return true;
        }
    }

    return false;
}

bool OrderModel::insertRows(int row, int count, QModelIndex const &parent) {
    if (mOrder != nullptr) {
        if (rowCount() + count >= 256) {
            return false;
        }
        
        beginInsertRows(parent, row, row + count);
        mOrder->insert(mOrder->cbegin() + row, count, { 0, 0, 0, 0 });
        endInsertRows();
        return true;
    }
    return false;
}

bool OrderModel::removeRows(int row, int count, QModelIndex const &parent) {
    if (mOrder != nullptr) {
        if (rowCount() - count < 0) {
            return false;
        }

        beginRemoveRows(parent, row, row + count);
        auto iter = mOrder->cbegin() + row;
        mOrder->erase(iter, iter + count);
        endRemoveRows();
        return true;
    }


    return false;
}

void OrderModel::incrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::incdec>(1, selection);
}

void OrderModel::decrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::incdec>(0, selection);
}

void OrderModel::setSelection(QItemSelection const &selection, uint8_t id) {
    modifySelection<ModifyMode::set>(id, selection);
}

void OrderModel::setOrder(std::vector<trackerboy::Order> *order) {
    beginResetModel();
    mOrder = order;
    endResetModel();
}

void OrderModel::insert() {
    insertRows(mCurrentRow, 1);
    if (rowCount() == 2) {
        mActions.remove->setEnabled(true);
    }
}

void OrderModel::remove() {
    removeRows(mCurrentRow, 1);
    if (rowCount() == 1) {
        mActions.remove->setEnabled(false);
    }
}

template <OrderModel::ModifyMode mode>
void OrderModel::modifySelection(int option, QItemSelection const &selection) {
    mDocument.lock();
    QItemSelectionRange range;
    foreach(range, selection) {
        QModelIndex index;
        foreach(index, range.indexes()) {
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
