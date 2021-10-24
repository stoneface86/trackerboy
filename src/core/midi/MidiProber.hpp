
#pragma once

#include "RtMidi.h"

#include <QStringList>

#include <memory>
#include <vector>


//
// Input MIDI device prober. Provides a list of all MIDI APIs available on the
// system and maintains an instance to RtMidiIn for a given API.
//
class MidiProber {


public:
    MidiProber(MidiProber const&) = delete;
    void operator=(MidiProber const&) = delete;

    int indexOfApi(RtMidi::Api api) const;

    RtMidi::Api api() const;

    QStringList backendNames() const;

    QStringList portNames() const;

    void setBackend(int index);
    
    std::shared_ptr<RtMidiIn> backend() const;

    void probe();
    
    static MidiProber& instance();

private:
    MidiProber();

    // sets the prober to use no backend
    void noBackend();

    std::vector<RtMidi::Api> mApiVec;
    QStringList mBackendNames;
    QStringList mPortNames;

    int mBackendIndex;
    std::shared_ptr<RtMidiIn> mBackend;
    

};
