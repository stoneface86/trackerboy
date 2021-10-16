
#include "core/commands/order.hpp"
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

