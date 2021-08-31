#pragma once

class PatternModel;

#include "core/clipboard/PatternClip.hpp"

#include <QUndoCommand>

#include <cstdint>

//
// Command for pasting pattern data
//
class PasteCmd : public QUndoCommand {

    PatternModel &mModel;
    PatternClip mSrc;
    PatternClip mPast;
    PatternCursor mPos;
    uint8_t mPattern;
    bool mMix;

public:
    PasteCmd(
        PatternModel &model,
        PatternClip const& clip,
        PatternCursor pos,
        bool mix
    );

    virtual void redo() override;

    virtual void undo() override;

};
