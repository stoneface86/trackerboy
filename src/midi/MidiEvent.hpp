
#pragma once

#include <QEvent>

//
// Custom event for a MIDI input message
//
class MidiEvent : public QEvent {

public:

    enum Message {
        NoteOn,
        NoteOff
    };

    explicit MidiEvent(Message msg, int note = -1);

    //
    // Get the type of the message that was received
    //
    Message message() const;

    //
    // Get the note pressed if the message type was NoteOn
    //
    int note() const;


private:

    Message const mMessage;
    int const mNote;


};

