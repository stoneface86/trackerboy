
#pragma once

#include "midi/MidiEnumerator.hpp"

#include "RtMidi.h"

#include <QMutex>
#include <QObject>
#include <QPointer>

#include <optional>
#include <vector>


//
// Midi class. Sends a MidiEvent whenever a MIDI message is received.
//
class Midi : public QObject {

    Q_OBJECT

public:

    explicit Midi(QObject *parent = nullptr);

    bool isOpen();

    QString lastError();

    //
    // Closes the open port and disables MIDI input
    //
    void close();

    //
    // Sets the port to receive MIDI messages from. true is returned
    // on success.
    //
    bool open(MidiEnumerator::Device const& device);

    //
    // Sets the object that will receive MIDI message events. The receiver can only
    // be set when there is no device open.
    //
    void setReceiver(QObject *obj);

    
signals:
    void error();

private:
    Q_DISABLE_COPY(Midi)


    // callback functions
    // note that these functions are called from a separate thread

    static void midiInCallback(double deltatime, std::vector<unsigned char> *message, void *userData);
    void handleMidiIn(double deltatime, std::vector<unsigned char> &message);

    static void midiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData);
    void handleMidiError(RtMidiError::Type type, const std::string &errorText);

    std::optional<RtMidiIn> mMidiIn;

    QMutex mMutex;
    // start of mutex requirement
    // mutex is required since these variables are modified from
    // the callback functions, which are called from an RtMidi managed thread
    QPointer<QObject> mReceiver;
    QString mLastErrorString;

    // MIDI pitch of the last note on message, we only send the noteOff signal
    // when we get a note off message with this pitch
    int mLastNotePitch;

    // end of mutex requirement


};
