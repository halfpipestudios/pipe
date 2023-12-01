#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"
#include "game.h"
#include "editor.h"

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();

    GraphicsManager::Get()->SetRasterizerState(RASTERIZER_STATE_CULL_NONE);
    
    Game game;
    game.Initialize();
    
    Editor editor;
    editor.Initialize(&game);
    
    bool editorIsActive = false;

    f64 lastTimer = PlatformManager::Get()->GetTimeInSeconds();
    while(PlatformManager::Get()->IsRunning()) {

        MemoryManager::Get()->ClearFrameMemory();

        f64 currentTime = PlatformManager::Get()->GetTimeInSeconds();
        f64 deltaTime = currentTime - lastTimer;
        lastTimer = currentTime;

        if(PlatformManager::Get()->OnResize()) {
            GraphicsManager::Get()->ResizeBuffers();
        }

        PlatformManager::Get()->PollEvents();

        if(PlatformManager::Get()->GetInput()->KeyJustPress(KEY_TAB)) editorIsActive = !editorIsActive;

        i32 w = PlatformManager::Get()->GetWindow()->GetWidth(); 
        i32 h = PlatformManager::Get()->GetWindow()->GetHeight(); 
        GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)w/(f32)h, 0.01f, 1000.0f));
        GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_POINT);
        GraphicsManager::Get()->SetViewport(0, 0, w, h);
        GraphicsManager::Get()->BindFrameBuffer(nullptr);
        //GraphicsManager::Get()->ClearColorBuffer(nullptr, 0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearColorBuffer(nullptr, 0.1f, 0.05f, 0.4f);
        GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);
        
        // TODO: game.FixUpdate() game.PostUpdate();
        
        if(editorIsActive) {
            editor.Update(deltaTime);
            editor.Render();
        } else {
            game.Update(deltaTime);
            game.Render();
        }

        GraphicsManager::Get()->Present(1);
    }

    editor.Terminate();
    game.Terminate();

    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
