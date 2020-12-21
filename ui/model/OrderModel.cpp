
#include "model/OrderModel.hpp"

#include <QColor>

#include <algorithm>


OrderModel::OrderModel(ModuleDocument &document, QObject *parent) :
    QAbstractTableModel(parent),
    mDocument(document),
    mOrder(nullptr),
    mCurrentRow(0),
    mCurrentTrack(0),
    mCanSelect(true)
{
}

void OrderModel::incrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::inc>(selection);
    emit patternsChanged();
}

void OrderModel::decrementSelection(QItemSelection const &selection) {
    modifySelection<ModifyMode::dec>(selection);
    emit patternsChanged();
}

void OrderModel::select(int row, int track) {
    if (mCanSelect) {
        selectPattern(row);
        selectTrack(track);
    }
}

void OrderModel::selectPattern(int pattern) {
    if (mCurrentRow != pattern) {
        doSelectPattern(pattern);
    }
}

void OrderModel::selectTrack(int track) {
    if (mCurrentTrack != track) {
        doSelectTrack(track);
    }
}

void OrderModel::setOrder(std::vector<trackerboy::Order> *order) {
    beginResetModel();
    mOrder = order;
    endResetModel();
    doSelectPattern(0);
    doSelectTrack(0);
}

void OrderModel::setSelection(QItemSelection const &selection, uint8_t id) {
    modifySelection<ModifyMode::set>(selection, id);
    emit patternsChanged();
}

void OrderModel::setRowColor(QColor const& color) {
    mRowColor = color;
    emit dataChanged(createIndex(mCurrentRow, 0, nullptr), createIndex(mCurrentRow, 3, nullptr), { Qt::BackgroundRole });
}

// model implementation

int OrderModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4; // 4 columns, 1 for each channel
}

QVariant OrderModel::data(const QModelIndex &index, int role) const {
    if (mOrder != nullptr) {
        switch (role) {

            case Qt::DisplayRole: {
                auto row = (*mOrder)[index.row()];
                int id = row.tracks[index.column()];
                return QString("%1").arg(id, 2, 16, QLatin1Char('0')).toUpper();
            }
            case Qt::TextAlignmentRole:
                return Qt::AlignCenter;
            case Qt::BackgroundRole:
                if (index.row() == mCurrentRow) {
                    return mRowColor;
                }
                break;
            default:
                break;
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
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    } else if (role == Qt::BackgroundRole) {
        if (orientation == Qt::Vertical && section == mCurrentRow) {
            return mRowColor;
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
        if (ok && id < trackerboy::Song::MAX_ORDERS) {
            {
                auto ctx = mDocument.beginEdit();
                auto &row = (*mOrder)[index.row()];
                row.tracks[index.column()] = static_cast<uint8_t>(id);
            }
            emit patternsChanged();
            return true;
        }
    }

    return false;
}

bool OrderModel::insertRows(int row, int count, QModelIndex const &parent) {
    Q_UNUSED(parent);

    if (mOrder != nullptr) {
        if (rowCount() + count > trackerboy::Song::MAX_ORDERS) {
            return false;
        }
        
        mCanSelect = false;
        beginInsertRows(parent, row, row + count - 1);
        {
            auto ctx = mDocument.beginEdit();
            mOrder->insert(mOrder->cbegin() + row, count, { 0, 0, 0, 0 });
        }
        endInsertRows();
        mCanSelect = true;

        int rows = rowCount();
        if (mCurrentRow == rows - 2) {
            emit patternsChanged();
        }
        // enforce the current selection
        emit currentIndexChanged(createIndex(mCurrentRow, mCurrentTrack, nullptr));

        emit canMoveUp(mCurrentRow != 0);
        emit canMoveDown(mCurrentRow != rows - 1);
        emit canRemove(true);
        emit canInsert(rows != trackerboy::Song::MAX_ORDERS);

        return true;
    }
    return false;
}

bool OrderModel::removeRows(int row, int count, QModelIndex const &parent) {
    Q_UNUSED(parent);

    if (mOrder != nullptr) {
        if (rowCount() - count <= 0) {
            return false;
        }

        int rowEnd = row + count;

        // ignore any selections during the remove
        mCanSelect = false;
        beginRemoveRows(parent, row, rowEnd - 1);
        {
            auto ctx = mDocument.beginEdit();
            auto iter = mOrder->cbegin() + row;
            mOrder->erase(iter, iter + count);
        }
        endRemoveRows();
        mCanSelect = true;

        auto rows = rowCount();
        if (mCurrentRow >= rows) {
            selectPattern(rows - 1);
        } else {
            emit patternsChanged(); // redraw
            // enforce the current selection
            emit currentIndexChanged(createIndex(mCurrentRow, mCurrentTrack, nullptr));

            emit canMoveUp(mCurrentRow != 0);
            emit canMoveDown(mCurrentRow != rows - 1);
        }

        // we can always insert after a remove
        emit canInsert(true);
        emit canRemove(rows != 1);

        return true;
    }


    return false;
}



// slots

void OrderModel::insert() {
    insertRows(mCurrentRow, 1);
}

void OrderModel::remove() {
    removeRows(mCurrentRow, 1);
}

void OrderModel::duplicate() {
    mCanSelect = false;
    beginInsertRows(QModelIndex(), mCurrentRow, mCurrentRow);
    {
        auto ctx = mDocument.beginEdit();
        mOrder->insert(mOrder->cbegin() + mCurrentRow, mOrder->at(mCurrentRow));
    }
    endInsertRows();
    mCanSelect = true;
}

void OrderModel::moveUp() {
    // moving a row up is just swapping the current row with the previous one
    {
        auto ctx = mDocument.beginEdit();
        auto iter = mOrder->begin() + mCurrentRow;
        std::iter_swap(iter, iter - 1);
    }
    emit dataChanged(
        createIndex(mCurrentRow - 1, 0, nullptr),
        createIndex(mCurrentRow, 3, nullptr),
        { Qt::DisplayRole }
    );
    select(mCurrentRow - 1, mCurrentTrack);
}

void OrderModel::moveDown() {
    {
        auto ctx = mDocument.beginEdit();
        auto iter = mOrder->begin() + mCurrentRow;
        std::iter_swap(iter, iter + 1);
    }
    emit dataChanged(
        createIndex(mCurrentRow, 0, nullptr),
        createIndex(mCurrentRow + 1, 3, nullptr),
        { Qt::DisplayRole }
    );
    select(mCurrentRow + 1, mCurrentTrack);
}

// private methods

template <OrderModel::ModifyMode mode>
void OrderModel::modifySelection(QItemSelection const &selection, uint8_t option) {
    auto ctx = mDocument.beginEdit();

    if (selection.isEmpty()) {
        // no selection, modify the current cell
        auto &cell = (*mOrder)[mCurrentRow].tracks[mCurrentTrack];
        modifyCell<mode>(cell, option);
        auto cellIndex = createIndex(mCurrentRow, mCurrentTrack, nullptr);
        emit dataChanged(cellIndex, cellIndex, { Qt::DisplayRole });
    } else {

        for (auto range : selection) {
            for (auto const &index : range.indexes()) {
                auto &id = (*mOrder)[index.row()].tracks[index.column()];
                modifyCell<mode>(id, option);

            }
            emit dataChanged(range.topLeft(), range.bottomRight(), { Qt::DisplayRole });
        }
    }
}

template <OrderModel::ModifyMode mode>
void OrderModel::modifyCell(uint8_t &cell, uint8_t value) {
    if constexpr (mode == ModifyMode::inc) {
        Q_UNUSED(value);
        if (cell != 255) {
            ++cell;
        }
    } else if constexpr (mode == ModifyMode::dec) {
        Q_UNUSED(value);
        if (cell != 0) {
            --cell;
        }
    } else {
        cell = value;
    }
}

void OrderModel::doSelectPattern(int pattern) {
    auto oldpattern = mCurrentRow;
    mCurrentRow = pattern;
    emit dataChanged(
        createIndex(oldpattern, 0, nullptr),
        createIndex(oldpattern, 3, nullptr),
        { Qt::BackgroundRole });
    emit dataChanged(
        createIndex(pattern, 0, nullptr),
        createIndex(pattern, 3, nullptr),
        { Qt::BackgroundRole });
    emit currentIndexChanged(createIndex(mCurrentRow, mCurrentTrack, nullptr));
    emit currentPatternChanged(pattern);

    emit canMoveUp(pattern != 0);
    emit canMoveDown(pattern != rowCount() - 1);
}

void OrderModel::doSelectTrack(int track) {
    auto oldtrack = mCurrentTrack;
    mCurrentTrack = track;

    emit dataChanged(
        createIndex(mCurrentRow, oldtrack, nullptr),
        createIndex(mCurrentRow, oldtrack, nullptr),
        { Qt::BackgroundRole });
    emit dataChanged(
        createIndex(mCurrentRow, track, nullptr),
        createIndex(mCurrentRow, track, nullptr),
        { Qt::BackgroundRole });

    emit currentIndexChanged(createIndex(mCurrentRow, mCurrentTrack, nullptr));
    emit currentTrackChanged(track);
}
