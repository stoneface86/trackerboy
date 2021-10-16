#pragma once

class PatternModel;

#include "trackerboy/data/OrderRow.hpp"

#include <QUndoCommand>

//
// Command for duplicating a row in the order
//
class OrderDuplicateCmd : public QUndoCommand {

public:

    explicit OrderDuplicateCmd(PatternModel &model, int row);

    virtual void redo() override;

    virtual void undo() override;

private:
    PatternModel &mModel;
    int mRow;

};

//
// Command for editing a row in the order
//
class OrderEditCmd : public QUndoCommand {

public:

    explicit OrderEditCmd(
        PatternModel &model,
        trackerboy::OrderRow newRow,
        int pattern
    );

    virtual void redo() override;

    virtual void undo() override;

private:

    void setData(trackerboy::OrderRow row);

    PatternModel &mModel;
    trackerboy::OrderRow const mOldRow;
    trackerboy::OrderRow const mNewRow;
    int const mPattern;

};

//
// Command for inserting an order after a given row.
//
class OrderInsertCmd : public QUndoCommand {

public:

    explicit OrderInsertCmd(PatternModel &model, int row);

    virtual void redo() override;

    virtual void undo() override;

private:

    PatternModel &mModel;
    int mRow;


};

//
// Command for removing a row in the order
//
class OrderRemoveCmd : public QUndoCommand {

public:

    explicit OrderRemoveCmd(PatternModel &model, int row);

    virtual void redo() override;

    virtual void undo() override;

private:

    PatternModel &mModel;
    trackerboy::OrderRow mRemovedRow;
    int mRow;

};

//
// Command for swapping two row indices (for moving up or moving down)
//
class OrderSwapCmd : public QUndoCommand {

public:

    explicit OrderSwapCmd(PatternModel &model, int from, int to);

    virtual void redo() override;

    virtual void undo() override;

private:

    void swap();

    PatternModel &mModel;
    int const mFrom;
    int const mTo;
};


