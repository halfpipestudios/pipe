#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"

int main() {
    
    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();

    while(PlatformManager::Get()->IsRunning()) {
        
        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();

        Sleep(0.016);
    }

    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
