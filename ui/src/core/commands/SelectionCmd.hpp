
#pragma once

class PatternModel;

#include "core/clipboard/PatternClip.hpp"

#include <QUndoCommand>

#include <cstdint>

//
// Base class for commands that operate on a PatternSelection
//
class SelectionCmd : public QUndoCommand {

protected:
    PatternModel &mModel;
    uint8_t mPattern;
    PatternClip mClip;

    //
    // initializes the command by saving a clip of the current selection
    //
    explicit SelectionCmd(PatternModel &model);

    //
    // Restores the saved clip
    //
    void restore(bool update);

};
