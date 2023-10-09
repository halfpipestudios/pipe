#include "platform_manager.h"

Win32Platform PlatformManager::win32;

Platform *PlatformManager::Get()
{
    return (Platform *)&win32;
}
