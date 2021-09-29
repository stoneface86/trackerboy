
#include "core/commands/order/OrderRemoveCmd.hpp"
#include "core/model/PatternModel.hpp"

OrderRemoveCmd::OrderRemoveCmd(PatternModel &model, int row) :
    mModel(model),
    mRemovedRow(model.order()[row]),
    mRow(row)
{
}

void OrderRemoveCmd::redo() {
    mModel.removeOrderImpl(mRow);
}

void OrderRemoveCmd::undo() {
    // to undo, re-insert the previously removed row
    mModel.insertOrderImpl(mRemovedRow, mRow);
}
