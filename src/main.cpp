#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"
#include "game.h"

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();

    Game game;

    game.Initialize();

    f64 lastTimer = PlatformManager::Get()->GetTimeInSeconds();
    u32 FPS = 0;
    while(PlatformManager::Get()->IsRunning()) {
        
        MemoryManager::Get()->ClearFrameMemory();

        f64 currentTime = PlatformManager::Get()->GetTimeInSeconds();
        f64 deltaTime = currentTime - lastTimer;
        lastTimer = currentTime;

        PlatformManager::Get()->PollEvents();
        
        game.Update(deltaTime);

        // TODO: game.FixUpdate() game.PostUpdate();

        GraphicsManager::Get()->ClearColorBuffer(0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearDepthStencilBuffer();

        game.Render();

        GraphicsManager::Get()->Present(1);
    }

    game.Terminate();

    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
