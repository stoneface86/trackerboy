
#pragma once

#include "miniaudio.h"

#include <QMutex>
#include <QStringList>

#include <memory>
#include <vector>

//
// Singleton class for managing all available miniaudio backends. A context to
// each available backend is stored in this instance, but are only initialized
// when needed (lazy loading). Note that device indices used by this class are
// offset by 1, as the index 0 is used to specify the default device.
//
// Not thread-safe. Should only be called from the GUI thread
//
class AudioProber {

public:
    AudioProber(AudioProber const&) = delete;
    void operator=(AudioProber const&) = delete;

    //
    // Gets the one and only instance
    //
    static AudioProber& instance();

    //
    // Gets the backend index of the RtAudio api. -1 is returned if the
    // index was not found.
    //
    int indexOfBackend(ma_backend backend) const;

    //
    // Gets the index of the device with the given name in the given backendIndex
    // -1 is returned if the device was not found. This function does not
    // probe the backend before searching, so be sure to probe before calling this
    // function.
    //
    int indexOfDevice(int backendIndex, ma_device_id const& id) const;

    ma_context* context(int backendIndex);

    ma_device_id* deviceId(int backendIndex, int deviceIndex);

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
    // Probes all available devices for the given backend
    //
    void probe(int backendIndex);

private:
    AudioProber();
    ~AudioProber() = default;

    bool indexIsInvalid(int backendIndex) const;


    class Context {
        
    public:

        Context() = delete;
        explicit Context(ma_backend backend);
        Context(const Context&) = delete;
        Context(Context&&) = default;
        ~Context();

        ma_backend backend() const;

        //
        // Gets the miniaudio context for this backend
        //
        ma_context* get();

        ma_device_id* id(int deviceIndex);

        int findDevice(ma_device_id const& id) const;

        QStringList deviceNames() const;

        //
        // Probe all devices in the rtaudio handle, updating ids and names
        //
        void probe();

    private:

        static ma_bool32 enumerateCallback(ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData);
        void enumerate(ma_device_info const* info);

        // a pointer is used for lazy loading
        // this way we only initialize the backends when they are used
        

        std::unique_ptr<ma_context> mContext;
        ma_backend const mBackend;

        std::vector<ma_device_info> mDevices;

        //ma_device_info *mDeviceInfos;
        //ma_uint32 mDeviceCount;

    };

    std::vector<Context> mContexts;

    //mutable QMutex mMutex;



};
