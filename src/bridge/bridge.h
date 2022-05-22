
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TbVersion {
        int major;
        int minor;
        int patch;
    } TbVersion;

    extern void NimMain();

    extern TbVersion tb_getAppVersion();


#ifdef __cplusplus
}
#endif

