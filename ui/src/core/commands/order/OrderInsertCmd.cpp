
#include "core/commands/order/OrderInsertCmd.hpp"
#include "core/model/PatternModel.hpp"

OrderInsertCmd::OrderInsertCmd(PatternModel &model, int row) :
    mModel(model),
    mRow(row)
{
}

void OrderInsertCmd::redo() {
    mModel.insertOrderImpl(mModel.order().nextUnused(), mRow + 1);
}

void OrderInsertCmd::undo() {
    // to undo an insert, we remove the inserted row
    mModel.removeOrderImpl(mRow + 1);
}
