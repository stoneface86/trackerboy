
#pragma once

class PatternModel;

#include "trackerboy/data/OrderRow.hpp"

#include <QUndoCommand>

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
