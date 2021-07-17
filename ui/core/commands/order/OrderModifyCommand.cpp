
#include "core/commands/order/OrderModifyCommand.hpp"


OrderModifyCommand::OrderModifyCommand(OrderModel &model, uint8_t pattern, uint8_t track, uint8_t newValue, uint8_t oldValue) :
        QUndoCommand(),
        mModel(model),
        mPattern(pattern),
        mTrack(track),
        mNewValue(newValue),
        mOldValue(oldValue)
{

}

void OrderModifyCommand::undo() {
    setCell(mOldValue);
}

void OrderModifyCommand::redo() {
    setCell(mNewValue);
}

void OrderModifyCommand::setCell(uint8_t value) {
    {
        auto ctx = mModel.mDocument.beginCommandEdit();
        mModel.mOrder[mPattern][mTrack] = value;
    }
    auto cellIndex = mModel.createIndex(mPattern, mTrack, nullptr);
    emit mModel.dataChanged(cellIndex, cellIndex, { Qt::DisplayRole });
}
