
#include "core/commands/order/OrderSwapCmd.hpp"
#include "core/model/PatternModel.hpp"


OrderSwapCmd::OrderSwapCmd(PatternModel &model, int from, int to) :
    mModel(model),
    mFrom(from),
    mTo(to)
{
}

void OrderSwapCmd::redo() {
    swap();
    mModel.setCursorPattern(mTo);
}

void OrderSwapCmd::undo() {
    swap();
    mModel.setCursorPattern(mFrom);
}

void OrderSwapCmd::swap() {
    auto &order = mModel.order();
    {
        auto editor = mModel.mModule.edit();
        order.swapPatterns(mFrom, mTo);
    }
}
