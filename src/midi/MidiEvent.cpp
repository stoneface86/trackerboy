
#include "midi/MidiEvent.hpp"
#include "CustomEvents.hpp"

MidiEvent::MidiEvent(Message msg, int note) :
    QEvent((QEvent::Type)CustomEvents::MidiEvent),
    mMessage(msg),
    mNote(note)
{

}

MidiEvent::Message MidiEvent::message() const {
    return mMessage;
}

int MidiEvent::note() const {
    return mNote;
}
