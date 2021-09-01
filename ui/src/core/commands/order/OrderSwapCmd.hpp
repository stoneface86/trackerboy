
#pragma once

class PatternModel;

#include <QUndoCommand>

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
