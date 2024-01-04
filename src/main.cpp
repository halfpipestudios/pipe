#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include "mgr/texture_manager.h"
#include "mgr/model_manager.h"
#include "mgr/animation_manager.h"

#include "game.h"
#include "edt/editor.h"

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();
    TextureManager::Get()->Initialize(256);
    ModelManager::Get()->Initialize(256);
    AnimationManager::Get()->Initialize(256);

    GraphicsManager::Get()->SetRasterizerState(RASTERIZER_STATE_CULL_NONE);
    
    Game game;
    game.Initialize();
    
    Editor editor;
    editor.Initialize(&game.level);

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
        GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_LINEAR);
        GraphicsManager::Get()->SetViewport(0, 0, w, h);
        GraphicsManager::Get()->BindFrameBuffer(nullptr);
        GraphicsManager::Get()->ClearColorBuffer(nullptr, 0.1f, 1.0f, 0.4f);
        GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);
        
        
        game.BeginFrame(deltaTime);
        if(editorIsActive) {
            editor.Update(deltaTime);
            editor.Render();
        } else {
            game.Update(deltaTime);
            game.Render();
        }
        GraphicsManager::Get()->Present(1);
        game.EndFrame(deltaTime);
    }

    editor.Terminate();
    game.Terminate();

    AnimationManager::Get()->Terminate();
    ModelManager::Get()->Terminate();
    TextureManager::Get()->Terminate();
    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
