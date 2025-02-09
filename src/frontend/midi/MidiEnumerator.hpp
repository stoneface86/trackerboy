#pragma once

#include "RtMidi.h"

#include <QStringList>
#include <QVariant>

#include <vector>

//
// Class for enumerating MIDI input devices. Devices are addressable via a backend (api) and device
// index, similiarly to AudioEnumerator.
//
class MidiEnumerator {

public:

    struct Device {

    private:

        Device();
        Device(RtMidi::Api api, std::string &&portName);

        friend class Midi;
        friend class MidiEnumerator;

        RtMidi::Api api;
        std::string portName;

    };


    explicit MidiEnumerator();

    bool backendIsAvailable(int backend) const;

    QStringList backendNames() const;

    int backends() const;

    QStringList deviceNames(int backend) const;

    int devices(int backend) const;

    Device device(int backend, int device) const;

    QVariant serializeDevice(int backend, int device) const;

    int deserializeDevice(int backend, QVariant const& data) const;

    void populate(int backend);

private:

    bool indexIsInvalid(int backend) const;

    struct Context {

        explicit Context(RtMidi::Api api);

        RtMidi::Api api;
        QStringList deviceNames;
        bool available;

    };

    std::vector<Context> mContexts;


};

