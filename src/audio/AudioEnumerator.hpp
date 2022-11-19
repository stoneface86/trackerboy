
#pragma once

#include <QStringList>
#include <QVariant>

#include "miniaudio.h"

#include <memory>
#include <vector>

//
// Audio device enumeration. Devices for each available API on the
// system can be queried using this class.
//
// Devices are addressed using an index for the backend (API) and an index for the device.
// Index 0 is known as the "default device". The actual device used is determined by the backend,
// and for some backends, allows automatic stream routing.
//
class AudioEnumerator {

public:

    struct Device {
        std::shared_ptr<ma_context> context;
        ma_device_id const* id;
    };

    explicit AudioEnumerator();


    bool backendIsAvailable(int backend) const;

    //
    // Gets a list of names for each backend.
    //
    QStringList backendNames() const;

    //
    // Gets the count of available backends on the system.
    //
    int backends() const;

    //
    // Gets a list of available device names from the given backend.
    //
    QStringList deviceNames(int backend) const;

    //
    // Gets the count of available devices for the given backend
    //
   int devices(int backend) const;

    //
    // Gets a reference to a device handle for the given device address.
    // This handle is temporary and should be used immediately.
    //
    Device device(int backend, int device) const;

    //
    // Populates the device list for the given backend. To rescan for changes in the
    // device list, call this method again.
    //
    void populate(int backend);

    //
    // Serialize the device so that it can be uniquely identified. The
    // result of this function can be written to file using a QSettings.
    //
    QVariant serializeDevice(int backend, int device) const;

    //
    // Get the index of the serialized device data, if it exists. If the
    // device could not be found, 0 is returned. If the given data is not
    // a valid device, -1 is returned.
    //
    int deserializeDevice(int backend, QVariant const& data) const;

private:

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

        ma_context* get() const;

        std::shared_ptr<ma_context> getShared() const;

        ma_device_id const* id(int deviceIndex) const;

        bool initialized() const;

        int findDevice(ma_device_id const& id) const;

        QStringList deviceNames() const;

        int devices() const;

        //
        // Probe all devices in the rtaudio handle, updating ids and names
        //
        void probe();

    private:

        static ma_bool32 enumerateCallback(ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData);
        void enumerate(ma_device_info const* info);

        // a pointer is used for lazy loading
        // this way we only initialize the backends when they are used
        // also shared_ptr cause the ma_contexts have a lifetime outside its container


        std::shared_ptr<ma_context> mContext;
        bool mInitialized;
        ma_backend const mBackend;

        std::vector<ma_device_info> mDevices;

    };

    QStringList mBackendNames;

    std::vector<Context> mContexts;

};
