
#pragma once

#include "core/Guarded.hpp"

#include "rtaudio/RtAudio.h"

#include <QMutex>
#include <QStringList>

#include <memory>
#include <vector>

//
// Singleton class for managing all available RtAudio backends. A handle to
// each available API is stored in this instance, but are only initialized
// when needed (lazy loading).
// 
// All functions in this class are thread safe.
//
class AudioProber {

public:
    AudioProber(AudioProber const&) = delete;
    void operator=(AudioProber const&) = delete;


    int getDefaultDevice(int backend) const;

    //
    // Gets the one and only instance
    //
    static AudioProber& instance();

    //
    // Gets the backend index of the RtAudio api. -1 is returned if the
    // index was not found.
    //
    int indexOfApi(RtAudio::Api api) const;

    //
    // Gets the index of the device with the given name in the given backendIndex
    // -1 is returned if the device was not found. This function does not
    // probe the backend before searching, so be sure to probe before calling this
    // function.
    //
    int indexOfDevice(int backendIndex, QString const& name) const;

    //
    // Gets an RtAudio handle for the given backend or nullptr if the index
    // is out of bounds.
    //
    Guarded<RtAudio>* backend(int backendIndex);

    RtAudio::Api backendApi(int backendIndex) const;

    //
    // Gets a list of names for all available apis.
    //
    QStringList backendNames() const;

    //
    // Gets a list of names for all usable devices for the given backend index.
    // Must call probe first otherwise an empty list is returned.
    //
    QStringList deviceNames(int backendIndex) const;

    //
    // Utility for getting the device name from a backend and device index.
    // An empty string is returned if the device does not exist.
    //
    QString deviceName(int backendIndex, int deviceIndex) const;

    //
    // Finds the device index from the device name
    //
    int findDevice(int backendIndex, QString const& name) const;

    //
    // Maps our backend and device index to an RtAudio device number. 0 is returned
    // if the device wasn't found or the backend is invalid.
    //
    unsigned mapDeviceIndex(int backendIndex, int deviceIndex) const;

    //
    // Probes all available devices for the given backend
    //
    void probe(int backendIndex);

private:
    AudioProber();
    ~AudioProber() = default;

    bool indexIsInvalid(int backendIndex) const;


    struct Backend {
        // a pointer is used for lazy loading
        // this way we only initialize the backends when they are used
        std::unique_ptr<Guarded<RtAudio>> rtaudio;
        RtAudio::Api const api;
        // maps our device index to an RtAudio device index
        std::vector<unsigned> deviceIds;

        // list of device names
        QStringList deviceNames;

        int defaultDevice;

        Backend(RtAudio::Api api);

        //
        // Gets the RtAudio handle for this backend
        //
        Guarded<RtAudio>* get();

        //
        // Probe all devices in the rtaudio handle, updating ids and names
        //
        void probe();

    };

    std::vector<Backend> mBackends;
    mutable QMutex mMutex;



};
