
#pragma once

#include "miniaudio.h"

#include <QStringList>

//
// Wrapper class for a miniaudio context, ma_context. Also handles device
// enumeration.
//
class Miniaudio {

public:

    Miniaudio();
    ~Miniaudio();

    //
    // Initialize the miniaudio context using the default settings. The result
    // of the initialization is returned.
    //
    ma_result init();

    //
    // Initialize the miniaudio context using the specified backend
    //
    ma_result init(ma_backend const backend);

    //
    // Gets the miniaudio context set by init()
    //
    ma_context* context();

    //
    // Returns the total number of devices available
    //
    unsigned deviceCount() const;

    //
    // Gets a list of device names to display to the user.
    //
    QStringList deviceNames();

    //
    // Gets the device id of the given device index. Index 0 is the default device.
    //
    ma_device_id* deviceId(int index);

    //
    // Returns the index in the device list for the given device id.
    // -1 is returned if the device was not found.
    //
    int lookupDevice(ma_device_id *id);

    //
    // Gets the device name for the given device index
    //
    QString deviceName(int index);

    //
    // User readable version of the device's id
    //
    QString deviceIdString(ma_device_id const& id);

    //
    // Name of the backend in use
    //
    QString backendName();

    //
    // Enumerates all devices in the initialized context again.
    //
    void rescan();

private:

    ma_result initImpl(ma_backend const *backends, ma_uint32 count);

    bool mInitialized;
    ma_context mContext;
    ma_device_info *mDeviceList;
    ma_uint32 mDeviceCount;

};
