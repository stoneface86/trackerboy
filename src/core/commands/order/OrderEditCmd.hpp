
#pragma once

class PatternModel;

#include "trackerboy/data/OrderRow.hpp"

#include <QUndoCommand>

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
