
#pragma once

//
// Interface for receiving MIDI input messages
//
class IMidiReceiver {


public:

    virtual void midiNoteOn(int note) = 0;

    virtual void midiNoteOff() = 0;

protected:
    IMidiReceiver() = default;
    virtual ~IMidiReceiver() = default;


};
