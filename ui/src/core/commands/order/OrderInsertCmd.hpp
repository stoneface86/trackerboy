
#pragma once

class PatternModel;

#include <QUndoCommand>

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
