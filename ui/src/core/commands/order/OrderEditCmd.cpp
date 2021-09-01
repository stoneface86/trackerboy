
#include "core/commands/order/OrderEditCmd.hpp"
#include "core/model/PatternModel.hpp"

OrderEditCmd::OrderEditCmd(PatternModel &model, trackerboy::OrderRow newRow, int pattern) :
    mModel(model),
    mOldRow(model.order()[pattern]),
    mNewRow(newRow),
    mPattern(pattern)
{
}

void OrderEditCmd::redo() {
    setData(mNewRow);
}

void OrderEditCmd::undo() {
    setData(mOldRow);
}

void OrderEditCmd::setData(trackerboy::OrderRow row) {
    {
        auto editor = mModel.mModule.edit();
        mModel.order()[mPattern] = row;
    }
    mModel.invalidate(mPattern, true);
}
