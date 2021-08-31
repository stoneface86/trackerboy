
#pragma once

class PatternModel;

#include "trackerboy/data/TrackRow.hpp"

#include <QUndoCommand>

#include <cstdint>

//
// Base command class for editing a column in a track row
//
class TrackEditCmd : public QUndoCommand {

protected:
    PatternModel &mModel;
    uint8_t const mTrack;
    uint8_t const mPattern;
    uint8_t const mRow;
    uint8_t const mNewData;
    uint8_t const mOldData;

public:
    explicit TrackEditCmd(
        PatternModel &model,
        uint8_t dataNew,
        uint8_t dataOld,
        QUndoCommand *parent = nullptr
    );

    virtual void redo() override;

    virtual void undo() override;

protected:
    trackerboy::TrackRow& getRow();

    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) = 0;

private:
    void setData(uint8_t data);

};

//
// Command for editing the note column in a TrackRow
//
class NoteEditCmd : public TrackEditCmd {

    using TrackEditCmd::TrackEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override;
};

//
// Command for editing the instrument column in a TrackRow
//
class InstrumentEditCmd : public TrackEditCmd {

    using TrackEditCmd::TrackEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data);

};

//
// Base command class for editing an effect column
//
class EffectEditCmd : public TrackEditCmd {

public:
    explicit EffectEditCmd(
        PatternModel &model,
        uint8_t effectNo,
        uint8_t newData,
        uint8_t oldData,
        QUndoCommand *parent = nullptr
    );

protected:
    uint8_t const mEffectNo;
};

//
// Command class for editing an effect type in a TrackRow
//
class EffectTypeEditCmd : public EffectEditCmd {

    using EffectEditCmd::EffectEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override;

};

//
// Command class for editing an effect parameter in a TrackRow
//
class EffectParamEditCmd : public EffectEditCmd {

    using EffectEditCmd::EffectEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override;

};


