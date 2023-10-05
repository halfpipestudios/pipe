#include "platform_selector.h"

Win32Platform PlatformSelector::win32;

Platform *PlatformSelector::Get()
{
    return (Platform *)&win32;
}
