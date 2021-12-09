
#include "midi/Midi.hpp"

#include "trackerboy/note.hpp"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QtDebug>
#include <QEvent>

#include <algorithm>

#define TU MidiTU
namespace TU {

constexpr auto LOG_PREFIX = "[Midi]";

static void logError(const char* str) {
    qCritical() << LOG_PREFIX << str;
}


static void logError(const char* str, RtMidiError const& err) {
    qCritical() << LOG_PREFIX << str << err.getMessage().c_str();
}

//
// Custom event for a MIDI input message
//
class MidiEvent : public QEvent {

public:

    enum Message {
        NoteOn,
        NoteOff
    };

    explicit MidiEvent(Message msg, int note = -1) :
        QEvent(getType()),
        mMessage(msg),
        mNote(note)
    {

    }

    //
    // Get the type of the message that was received
    //
    Message message() const {
        return mMessage;
    }

    //
    // Get the note pressed if the message type was NoteOn
    //
    int note() const {
        return mNote;
    }

    static constexpr Type getType() {
        // since this event is only used internally by the Midi class, just
        // use the first User event id. 
        return QEvent::User;
    }

private:
    Message const mMessage;
    int const mNote;


};

}


Midi::Midi(QObject *parent) :
    QObject(parent),
    mReceiver(nullptr),
    mNoteDown(false),
    mMidiIn(),
    mMutex(),
    mLastNotePitch(-1)
{
}


bool Midi::isOpen() {
    return mMidiIn && mMidiIn->isPortOpen();
}

QString Midi::lastError() {
    QMutexLocker locker(&mMutex);
    return mLastErrorString;
}

void Midi::close() {
    if (isOpen()) {
        try {
            // remove callbacks
            mMidiIn->cancelCallback();
            mMidiIn->setErrorCallback();
            // close the current opened port
            mMidiIn->closePort();
        } catch (RtMidiError const& err) {
            TU::logError("could not close port:", err);
            return;
        }
    }
}

bool Midi::open(MidiEnumerator::Device const& device) {
    close();

    // (re)initialize the RtMidiIn object if
    //  - it has not been initialized yet
    //  - the device's api does not match the current one
    if (!mMidiIn || mMidiIn->getCurrentApi() != device.api) {
        try {
            mMidiIn.emplace(device.api);
        } catch (RtMidiError const& err) {
            mMidiIn.reset();
            TU::logError("could not initialize api:", err);
            return false;
        }
    }

    try {

        // find the device by name
        auto const port = [](RtMidiIn &midi, std::string const& name) {
            auto portCount = midi.getPortCount();
            for (unsigned port = 0; port < portCount; ++port) {
                if (name == midi.getPortName(port)) {
                    return (int)port;
                }
            }
            return -1;
        }(*mMidiIn, device.portName);

        if (port == -1) {
            TU::logError("could not find device");
            return false;
        }


        mLastNotePitch = -1;
        // setup callbacks and open the port
        mMidiIn->setCallback(midiInCallback, this);
        mMidiIn->openPort(port);
        // set the callback after opening, this way if we fail to open the error will be handled here
        mMidiIn->setErrorCallback(midiErrorCallback, this);
    } catch (RtMidiError const& err) {
        TU::logError("could not open port:", err.getMessage());
        return false;
    }

    return true;

}

void Midi::setReceiver(IMidiReceiver *receiver) {

    if (mReceiver != receiver) {
        // change the receiver
        if (mNoteDown && mReceiver) {
            // force the note off
            // if we don't do this, the previous receiver won't get the next noteOff message
            // and the note will be held indefinitely
            mReceiver->midiNoteOff();
            mNoteDown = false;
        }
        mReceiver = receiver;

    }
}

void Midi::customEvent(QEvent *evt) {
    if (evt->type() == TU::MidiEvent::getType()) {
        auto midiEvt = static_cast<TU::MidiEvent*>(evt);
        switch (midiEvt->message()) {
            case TU::MidiEvent::NoteOff:
                if (mReceiver) {
                    mReceiver->midiNoteOff();
                    mNoteDown = false;
                }
                break;
            case TU::MidiEvent::NoteOn:
                if (mReceiver) {
                    mReceiver->midiNoteOn(midiEvt->note());
                    mNoteDown = true;
                }
                break;
            default:
                break;
        }

    }
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

    auto const msgSize = message.size();
    if (msgSize == 0) {
        return; // shouldn't happen but just in case do nothing
    }
    
    auto const status = message[0];
    switch (status & 0xF0) {
        case MidiNoteOff:
            // message should be three bytes
            if (msgSize == 3) {
                if (mLastNotePitch == (int)message[1]) {
                    mLastNotePitch = -1;
                    QCoreApplication::postEvent(this, new TU::MidiEvent(TU::MidiEvent::NoteOff), Qt::HighEventPriority);
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
                QCoreApplication::postEvent(this, new TU::MidiEvent(TU::MidiEvent::NoteOn, trackerboyNote), Qt::HighEventPriority);
            }
            break;
        default:
            break; // ignore everything else
    }

}

void Midi::midiErrorCallback(RtMidiError::Type type, const std::string &errorText, void *userData) {
    static_cast<Midi*>(userData)->handleMidiError(type, errorText);
}

void Midi::handleMidiError(RtMidiError::Type type, const std::string &errorText) {

    mMutex.lock();
    mLastErrorString = QString::fromStdString(errorText);
    mMutex.unlock();

    if (type != RtMidiError::WARNING) {
        emit error();
    }

}

#undef TU
