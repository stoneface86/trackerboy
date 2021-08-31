
#pragma once

class PatternModel;

#include "core/commands/SelectionCmd.hpp"

//
// Command for erasing the contents of a selection
//
class EraseCmd : public SelectionCmd {

public:

    EraseCmd(PatternModel &model);

    virtual void redo() override;

    virtual void undo() override;

};
