
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
    // Gets the miniaudio context set by init()
    //
    ma_context* context();

    //
    // Gets a list of device names to display to the user.
    //
    QStringList deviceNames();

    //
    // Gets the device id of the given device index
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
    QString deviceIdString(int index);

    ma_device_info const* deviceInfo(int index);

    QString backendName();

private:

    bool mInitialized;
    ma_context mContext;
    ma_device_info *mDeviceList;
    ma_uint32 mDeviceCount;

};
