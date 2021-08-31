
#pragma once

class PatternModel;

#include "core/PatternSelection.hpp"

#include <QUndoCommand>

#include <cstdint>

//
// Command for reversing the contents of a selection. While reverse applies to
// a selection, we do not inherit from SelectionCmd, as its
// redo/undo actions are the same (reversing is an involutory function). So
// there is no need to save a chunk of the selection for undo'ing
//
class ReverseCmd : public QUndoCommand {

    PatternModel &mModel;
    PatternSelection mSelection;
    uint8_t mPattern;

public:

    explicit ReverseCmd(PatternModel &model);

    virtual void redo() override;

    virtual void undo() override;

private:

    void reverse();

};
