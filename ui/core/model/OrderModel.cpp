
#include "core/model/OrderModel.hpp"
#include "core/model/ModuleDocument.hpp"

#include <QColor>

#include <algorithm>

static QColor ROW_COLOR;

//
// Command that changes a single track id in the order list. 
//
class OrderModifyCommand : public QUndoCommand {
public:
    OrderModifyCommand(OrderModel &model, uint8_t pattern, uint8_t track, uint8_t newValue, uint8_t oldValue) :
        QUndoCommand(),
        mModel(model),
        mPattern(pattern),
        mTrack(track),
        mNewValue(newValue),
        mOldValue(oldValue)
    {

    }

    void undo() override {
        setCell(mOldValue);
    }

    void redo() override {
        setCell(mNewValue);
    }

private:

    void setCell(uint8_t value) {
        {
            auto ctx = mModel.mDocument.beginCommandEdit();
            mModel.mOrder[mPattern][mTrack] = value;
        }
        auto cellIndex = mModel.createIndex(mPattern, mTrack, nullptr);
        emit mModel.dataChanged(cellIndex, cellIndex, { Qt::DisplayRole });
    }
    
    OrderModel &mModel;
    uint8_t const mPattern;
    uint8_t const mTrack;
    uint8_t const mNewValue; // set on redo
    uint8_t const mOldValue; // set on undo

};

//
// Command that swaps two rows in the order list, used by move up and move down
//
class OrderSwapCommand : public QUndoCommand {

public:

    OrderSwapCommand(OrderModel &model, uint8_t row1, uint8_t row2) :
        QUndoCommand(),
        mModel(model),
        mRowFrom(row1),
        mRowTo(row2)
    {
    }

    void undo() override {
        mModel.cmdSwapRows(mRowFrom, mRowTo);
        mModel.select(mRowFrom, mModel.mCurrentTrack);
    }

    void redo() override {
        mModel.cmdSwapRows(mRowFrom, mRowTo);
        mModel.select(mRowTo, mModel.mCurrentTrack);
    }


private:

    OrderModel &mModel;
    uint8_t const mRowFrom; // row index we are moving from
    uint8_t const mRowTo;   // row index we are moving to

};

class OrderInsertCommand : public QUndoCommand {
public:
    OrderInsertCommand(OrderModel &model, uint8_t row, uint8_t count) :
        QUndoCommand(),
        mModel(model),
        mRow(row),
        mCount(count)
    {
    }

    void redo() override {
        mModel.cmdInsertRows(mRow, mCount);
    }

    void undo() override {
        // remove the previously added rows
        mModel.cmdRemoveRows(mRow, mCount);
    }

private:
    OrderModel &mModel;
    uint8_t const mRow;
    uint8_t const mCount;

};

class OrderRemoveCommand : public QUndoCommand {
public:
    OrderRemoveCommand(OrderModel &model, uint8_t row, uint8_t count) :
        QUndoCommand(),
        mModel(model),
        mRow(row),
        mCount(count),
        mRowData(new trackerboy::OrderRow[count])
    {
        // keep a copy of the rows we removed
        std::copy_n(mModel.mOrder.data().begin() + row, count, mRowData.get());
    }

    void redo() override {
        mModel.cmdRemoveRows(mRow, mCount);
    }

    void undo() override {
        // add back the previously removed rows
        mModel.cmdInsertRows(mRow, mCount, mRowData.get());
    }

private:
    OrderModel &mModel;
    uint8_t const mRow;
    uint8_t const mCount;
    std::unique_ptr<trackerboy::OrderRow[]> mRowData;
};

class OrderDuplicateCommand : public QUndoCommand {

public:
    OrderDuplicateCommand(OrderModel &model, uint8_t row) :
        QUndoCommand(),
        mModel(model),
        mRow(row)
    {
    }

    void redo() override {
        auto toDuplicate = mModel.mOrder[mRow];
        mModel.cmdInsertRows(mRow, 1, &toDuplicate);
    }

    void undo() override {
        mModel.cmdRemoveRows(mRow, 1);
    }

private:
    OrderModel &mModel;
    uint8_t const mRow;

};



OrderModel::OrderModel(ModuleDocument &document, QObject *parent) :
    QAbstractTableModel(parent),
    mDocument(document),
    mOrder(document.mod().song().order()),
    mCurrentRow(0),
    mCurrentTrack(0),
    mCanSelect(true)
{
}

ModuleDocument& OrderModel::document() {
    return mDocument;
}

uint8_t OrderModel::currentPattern() {
    return mCurrentRow;
}

QModelIndex OrderModel::currentIndex() {
    return createIndex(mCurrentRow, mCurrentTrack, nullptr);
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
    Q_ASSERT(pattern >= 0 && pattern < mOrder.size());
    auto patternU8 = (uint8_t)pattern;
    if (mCurrentRow != patternU8) {
        doSelectPattern(patternU8);
    }
}

void OrderModel::selectTrack(int track) {
    Q_ASSERT(track >= 0 && track < 4);
    auto trackU8 = (uint8_t)track;
    if (mCurrentTrack != trackU8) {
        doSelectTrack(trackU8);
    }
}

void OrderModel::reload() {
    beginResetModel();
    endResetModel();
    doSelectPattern(0);
    doSelectTrack(0);
}

void OrderModel::setSelection(QItemSelection const &selection, uint8_t id) {
    modifySelection<ModifyMode::set>(selection, id);
    emit patternsChanged();
}

void OrderModel::setRowColor(QColor const& color) {
    ROW_COLOR = color;
}

// model implementation

int OrderModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4; // 4 columns, 1 for each channel
}

QVariant OrderModel::data(const QModelIndex &index, int role) const {

    switch (role) {

        case Qt::DisplayRole: {
            auto row = mOrder[(uint8_t)index.row()];
            int id = (int)row[index.column()];
            return QString("%1").arg(id, 2, 16, QLatin1Char('0')).toUpper();
        }
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::BackgroundRole:
            if (index.row() == mCurrentRow) {
                return ROW_COLOR;
            }
            break;
        default:
            break;
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
            return ROW_COLOR;
        }
    }
    return QVariant();
}

int OrderModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return (int)mOrder.size();
}

bool OrderModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {

        bool ok;
        unsigned id = value.toString().toUInt(&ok, 16);
        if (ok && id < trackerboy::MAX_PATTERNS) {
            auto row = index.row();
            auto track = index.column();

            auto cmd = new OrderModifyCommand(
                *this, 
                (uint8_t)row,
                (uint8_t)track,
                (uint8_t)id, 
                mOrder[(uint8_t)row][track]
            );
            cmd->setText(tr("Set track in pattern #%1\nSet track").arg(row));
            mDocument.undoStack().push(cmd);

            emit patternsChanged();
            return true;
        }
    }

    return false;
}

// slots

void OrderModel::insert() {
    auto cmd = new OrderInsertCommand(*this, mCurrentRow, 1);
    cmd->setText(tr("Insert pattern at #%1\nInsert pattern").arg(mCurrentRow));
    mDocument.undoStack().push(cmd);
}

void OrderModel::remove() {
    auto cmd = new OrderRemoveCommand(*this, mCurrentRow, 1);
    cmd->setText(tr("Remove pattern #%1\nRemove pattern").arg(mCurrentRow));
    mDocument.undoStack().push(cmd);
}

void OrderModel::duplicate() {
    auto cmd = new OrderDuplicateCommand(*this, mCurrentRow);
    cmd->setText(tr("Duplicate pattern #%1\nDuplicate order").arg(mCurrentRow));
    mDocument.undoStack().push(cmd);
}

void OrderModel::moveUp() {
    // moving a row up is just swapping the current row with the previous one
    auto prev = (uint8_t)(mCurrentRow - 1);
    auto cmd = new OrderSwapCommand(*this, mCurrentRow, prev);
    cmd->setText(tr("Move pattern #%1 -> #%2\nMove pattern up").arg(mCurrentRow).arg(prev));
    mDocument.undoStack().push(cmd);
}

void OrderModel::moveDown() {
    auto next = (uint8_t)(mCurrentRow + 1);
    auto cmd = new OrderSwapCommand(*this, mCurrentRow, next);
    cmd->setText(tr("Move pattern #%1 -> #%2\nMove pattern down").arg(mCurrentRow).arg(next));
    mDocument.undoStack().push(cmd);

}

void OrderModel::setPatternCount(int count) {
    Q_ASSERT(count > 0 && count <= 256);
    QUndoCommand *cmd = nullptr;

    auto const currentCount = rowCount();
    if (count > currentCount) {
        // grow
        cmd = new OrderInsertCommand(*this, (uint8_t)currentCount, (uint8_t)(count - currentCount));
    } else if (count < currentCount) {
        // shrink
        auto amount = (uint8_t)(currentCount - count);
        cmd = new OrderRemoveCommand(*this, (uint8_t)(currentCount - amount), amount);
    }

    if (cmd != nullptr) {
        cmd->setText(tr("Set pattern count to %1\nSet patterns").arg(count));
        mDocument.undoStack().push(cmd);
    }
}

// private methods


template <OrderModel::ModifyMode mode>
void OrderModel::modifySelection(QItemSelection const &selection, uint8_t option) {
    auto &stack = mDocument.undoStack();

    QString operationStr;
    if constexpr (mode == ModifyMode::set) {
        operationStr = tr("Order set selection");
    } else if constexpr (mode == ModifyMode::inc) {
        operationStr = tr("Order increment selection");
    } else {
        operationStr = tr("Order decrement selection");
    }


    stack.beginMacro(operationStr);

    if (selection.isEmpty()) {
        // no selection, modify the current cell
        modifyCell<mode>(stack, mCurrentRow, mCurrentTrack, option);
    } else {
        // modify all cells in the selection
        for (auto range : selection) {
            for (auto const &index : range.indexes()) {
                modifyCell<mode>(stack, (uint8_t)index.row(), (uint8_t)index.column(), option);
            }
        }
    }

    stack.endMacro();
}

template <OrderModel::ModifyMode mode>
void OrderModel::modifyCell(QUndoStack &stack, uint8_t pattern, uint8_t track, uint8_t option) {
    auto oldVal = mOrder[pattern][track];
    uint8_t newVal;
    if constexpr (mode == ModifyMode::inc) {
        newVal = (oldVal == 255) ? oldVal : oldVal + 1;
    } else if constexpr (mode == ModifyMode::dec) {
        newVal = (oldVal == 0) ? oldVal : oldVal - 1;
    } else {
        newVal = option;
    }
    if (newVal != oldVal) {
        stack.push(new OrderModifyCommand(*this, pattern, track, newVal, oldVal));
    }

}

void OrderModel::cmdInsertRows(uint8_t row, uint8_t count, trackerboy::OrderRow *rowdata) {
    mCanSelect = false;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    {
        auto ctx = mDocument.beginCommandEdit();

        auto &vec = mOrder.data();
        auto dest = vec.begin() + row;
        if (rowdata != nullptr) {
            vec.insert(dest, rowdata, rowdata + count);
        } else {
            vec.insert(dest, count, { 0, 0, 0, 0 });
        }
        
    }
    endInsertRows();
    mCanSelect = true;

    int rows = rowCount();
    if (mCurrentRow == rows - 2) {
        emit patternsChanged();
    }
    // enforce the current selection
    emit currentIndexChanged(createIndex(mCurrentRow, mCurrentTrack, nullptr));
}

void OrderModel::cmdRemoveRows(uint8_t row, uint8_t count) {
    int rowEnd = row + count;
    
    // ignore any selections during the remove
    mCanSelect = false;
    beginRemoveRows(QModelIndex(), row, rowEnd - 1);
    {
        auto ctx = mDocument.beginCommandEdit();
        mOrder.remove(row, count);
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
    }

}

void OrderModel::cmdSwapRows(uint8_t from, uint8_t to) {
    {
        auto ctx = mDocument.beginCommandEdit();
        mOrder.swapPatterns(from, to);
    }

    auto fromIndex = createIndex(from, 0, nullptr);
    auto toIndex = createIndex(to, 0, nullptr);

    emit dataChanged(fromIndex, fromIndex, { Qt::DisplayRole });
    emit dataChanged(toIndex, toIndex, { Qt::DisplayRole });
}


void OrderModel::doSelectPattern(uint8_t pattern) {
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

}

void OrderModel::doSelectTrack(uint8_t track) {
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

bool OrderModel::canInsert() {
    return rowCount() < trackerboy::MAX_PATTERNS;
}

bool OrderModel::canRemove() {
    return rowCount() > 1;
}

bool OrderModel::canMoveUp() {
    return mCurrentRow != 0;
}

bool OrderModel::canMoveDown() {
    return mCurrentRow != rowCount() - 1;
}