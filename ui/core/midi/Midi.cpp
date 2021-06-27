
#include "core/midi/Midi.hpp"

#include "trackerboy/note.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <utility>

Midi::Midi(QObject *parent) :
    QObject(parent),
    mMidiIn(nullptr),
    mDevice(-1),
    mMutex(),
    mEnabled(false),
    mLastError(RtMidiError::UNSPECIFIED),
    mLastNotePitch(-1)
{
}

Midi::~Midi() {

}


bool Midi::isEnabled() {
    QMutexLocker locker(&mMutex);
    return mEnabled;
}

RtMidiError::Type Midi::lastError() {
    QMutexLocker locker(&mMutex);
    return mLastError;
}

QString Midi::lastErrorString() {
    QMutexLocker locker(&mMutex);
    return mLastErrorString;
}

void Midi::close() {
    setDevice(nullptr, 0);
}


bool Midi::setDevice(std::shared_ptr<RtMidiIn> midi, int index) {
    Q_ASSERT(index >= 0);

    // close any active port
    if (mMidiIn) {
        auto ptr = std::exchange(mMidiIn, nullptr);
        try {
            // remove callbacks
            ptr->cancelCallback();
            ptr->setErrorCallback();
            // close the current opened port
            ptr->closePort();
        } catch (RtMidiError const& err) {
            handleMidiError(err.getType(), err.getMessage(), false);
            return false;
        }
    }

    // open the new port
    if (midi) {
        try {
            mLastNotePitch = -1;
            // setup callbacks and open the port
            midi->setCallback(midiInCallback, this);
            midi->openPort(index);
            // set the callback after opening, this way if we fail to open the error will be handled here
            midi->setErrorCallback(midiErrorCallback, this);
        } catch (RtMidiError const& err) {
            handleMidiError(err.getType(), err.getMessage(), false);
            return false;
        }
    }
    

    mMidiIn = std::move(midi);
    mDevice = index;

    mMutex.lock();
    mEnabled = mMidiIn != nullptr;
    mMutex.unlock();

    return true;
}

// enum for midi status bytes
enum MidiStatus {
    // 0x00 - 0x7F : invalid
    MidiNoteOff = 0x80,
    MidiNoteOn = 0x90,
    MidiAftertouch = 0xA0,
    MidiController = 0xB0,
    MidiProgramChange = 0xC0,
    MidiChannelPressure = 0xD0,
    MidiPitch = 0xE0,
    MidiSysex = 0xF0,
    MidiQuarterFrame = 0xF1,
    MidiSongPosition = 0xF2,
    MidiSongSelect = 0xF3,
    // ? = 0xF4
    // ? = 0xF5
    MidiTuneRequest = 0xF6,
    // ? = 0xF7
    MidiClock = 0xF8,
    // ? = 0xF9
    MidiStart = 0xFA,
    MidiContinue = 0xFB,
    MidiStop = 0xFC,
    // ? = 0xFD
    MidiActiveSense = 0xFE,
    MidiReset = 0xFF
};

void Midi::midiInCallback(double deltatime, std::vector<unsigned char> *message, void *userdata) {
    if (message == nullptr) {
        return; // shouldn't happen but just in case do nothing
    }
    static_cast<Midi*>(userdata)->handleMidiIn(deltatime, *message);
}

void Midi::handleMidiIn(double deltatime, std::vector<unsigned char> &message) {
    Q_UNUSED(deltatime)

    auto msgSize = message.size();
    if (msgSize == 0) {
        return; // shouldn't happen but just in case do nothing
    }

    // This handler was written to support parsing of any MIDI message,
    // however, we only really care about note on and note off messages,
    // so we only handle those messages. Everything else is ignored.
    // also the velocity of the note is ignored as there is nothing we can do with it
    
    auto const status = message[0];
    bool const isSystem = status >= 0xF0;
    if (isSystem) {
        // ignore this
        return;
    } else {
        // message has an associated MIDI channel in the lower nibble of the status byte
        //int channel = status & 0xF;

        switch (status & 0xF0) {
            case MidiNoteOff:
                // message should be three bytes
                if (msgSize == 3) {
                    if (mLastNotePitch == (int)message[1]) {
                        mLastNotePitch = -1;
                        emit noteOff();
                    }
                }
                break;
            case MidiNoteOn:
                // message should be three bytes
                if (msgSize == 3) {
                    mLastNotePitch = (int)message[1];
                    // 69 is A-4
                    // 36 is C-2
                    int trackerboyNote = std::clamp((int)message[1] - 36, 0, (int)trackerboy::NOTE_LAST);
                    emit noteOn(trackerboyNote);
                }
                break;
            default:
                break; // ignore everything else
        }
    }
}

void Midi::midiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
    static_cast<Midi*>(userData)->handleMidiError(type, errorText);
}

void Midi::handleMidiError(RtMidiError::Type type, const std::string &errorText, bool emitError) {
    mMutex.lock();
    mLastError = type;
    mEnabled = false;
    mLastErrorString = QString::fromStdString(errorText);
    mMutex.unlock();

    qCritical().noquote() << "[MIDI]" << mLastErrorString;

    if (emitError) {
        emit error(mLastError);
    }

}

