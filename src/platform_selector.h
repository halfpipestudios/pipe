#ifndef _PLATFORM_SELECTOR_
#define _PLATFORM_SELECTOR_

#include "win32_platform.h"

struct PlatformSelector {
    static Platform* Get();
private:
    static Win32Platform win32;
    //LinuxPlatform linux
    //MacPlatform mac;
};

#endif
