
#pragma once

class PatternModel;

#include "core/commands/pattern/SelectionCmd.hpp"

#include <cstdint>

//
// Command for transposing all notes in a selection by a given semitone offset.
//
class TransposeCmd : public SelectionCmd {

    int8_t mTransposeAmount; // semitones to transpose all notes by

public:

    explicit TransposeCmd(PatternModel &model, int8_t transposeAmount);

    virtual void redo() override;

    virtual void undo() override;

};
