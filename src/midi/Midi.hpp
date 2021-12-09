
#pragma once

#include "midi/MidiEnumerator.hpp"
#include "midi/IMidiReceiver.hpp"

#include "RtMidi.h"

#include <QMutex>
#include <QObject>

#include <optional>
#include <vector>


//
// Midi class. Notifies an IMidiReceiver whenever a MIDI note message
// is received.
//
class Midi : public QObject {

    Q_OBJECT

public:

    explicit Midi(QObject *parent = nullptr);

    //
    // Determine if the port is open.
    //
    bool isOpen();

    //
    // RtMidi error string of the last error, empty for no error.
    //
    QString lastError();

    //
    // Closes the open port and disables MIDI input
    //
    void close();

    //
    // Opens the port to receive MIDI messages from. true is returned
    // on success.
    //
    bool open(MidiEnumerator::Device const& device);

    //
    // Set the receiver that will respond to midi note on/off messages
    //
    void setReceiver(IMidiReceiver *receiver);

    
signals:
    //
    // emitted if an error occurs during input
    //
    void error();

protected:

    virtual void customEvent(QEvent *evt) override;

private:
    Q_DISABLE_COPY(Midi)

    IMidiReceiver *mReceiver;
    bool mNoteDown;

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
    QString mLastErrorString;

    // MIDI pitch of the last note on message, we only send the noteOff signal
    // when we get a note off message with this pitch
    int mLastNotePitch;

    // end of mutex requirement


};
