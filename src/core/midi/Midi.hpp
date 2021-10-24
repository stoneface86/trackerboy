
#pragma once

#include "RtMidi.h"

#include <QMutex>
#include <QObject>

#include <memory>
#include <vector>


//
// Midi object emits signals whenever a MIDI message is received.
//
class Midi : public QObject {

    Q_OBJECT

public:

    explicit Midi(QObject *parent = nullptr);
    virtual ~Midi();

    bool isEnabled();

    RtMidiError::Type lastError();

    QString lastErrorString();

    //
    // Closes the open port and disables MIDI input
    //
    void close();

    //
    // Sets the port to receive MIDI messages from. true is returned
    // on success.
    //
    bool setDevice(std::shared_ptr<RtMidiIn> midi, int index);

    
signals:
    void error(int type);

    // midi message signals

    void noteOn(int note);
    void noteOff();

    //void noteOn(double deltatime, int channel, int pitch, int velocity);
    //void noteOff(double deltatime, int channel, int pitch, int velocity);

private:
    Q_DISABLE_COPY(Midi)

   

    // callback functions
    // note that these functions are called from a separate thread

    static void midiInCallback(double deltatime, std::vector<unsigned char> *message, void *userData);
    void handleMidiIn(double deltatime, std::vector<unsigned char> &message);

    static void midiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData);
    void handleMidiError(RtMidiError::Type type, const std::string &errorText, bool emitError = true);

    std::shared_ptr<RtMidiIn> mMidiIn;

    // -1 for no device
    int mDevice;


    QMutex mMutex;
    // start of mutex requirement
    // mutex is required since these variables are modified from
    // the callback functions, which are called from an RtMidi managed thread
    bool mEnabled;
    RtMidiError::Type mLastError;
    QString mLastErrorString;

    // MIDI pitch of the last note on message, we only send the noteOff signal
    // when we get a note off message with this pitch
    int mLastNotePitch;

    // end of mutex requirement


};
