
#pragma once

class PatternModel;

#include <QUndoCommand>

class OrderDuplicateCmd : public QUndoCommand {

public:

    explicit OrderDuplicateCmd(PatternModel &model, int row);

    virtual void redo() override;

    virtual void undo() override;

private:
    PatternModel &mModel;
    int mRow;

};
