
#include "core/commands/order/OrderDuplicateCmd.hpp"
#include "core/model/PatternModel.hpp"


OrderDuplicateCmd::OrderDuplicateCmd(PatternModel &model, int row) :
    mModel(model),
    mRow(row)
{
}

void OrderDuplicateCmd::redo() {
    mModel.insertOrderImpl(mModel.order()[mRow], mRow + 1);
}

void OrderDuplicateCmd::undo() {
    mModel.removeOrderImpl(mRow + 1);
}
