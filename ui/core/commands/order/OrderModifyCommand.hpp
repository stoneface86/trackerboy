
#pragma once

#include <QUndoCommand>

class ModuleDocument;
class OrderModel;


class OrderModifyCommand : public QUndoCommand {

public:
    OrderModifyCommand(OrderModel &model, uint8_t pattern, uint8_t track, uint8_t newValue, uint8_t oldValue);

    virtual void undo() override;

    virtual void redo() override;

private:

    void setCell(uint8_t value);

    OrderModel &mModel;
    uint8_t const mPattern;
    uint8_t const mTrack;
    uint8_t const mNewValue; // set on redo
    uint8_t const mOldValue; // set on undo
};
