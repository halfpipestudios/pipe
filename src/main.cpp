#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

static Vertex gQuad[] = {
    // Face 1
    {{-0.5f, -0.5f, 0}, {0, 0, 1}, {0, 1}},
    {{-0.5f,  0.5f, 0}, {0, 0, 1}, {0, 0}},
    {{ 0.5f, -0.5f, 0}, {0, 0, 1}, {1, 1}},
    // Face 2
    {{ 0.5f, -0.5f, 0}, {0, 0, 1}, {1, 1}},
    {{-0.5f,  0.5f, 0}, {0, 0, 1}, {0, 0}},
    {{ 0.5f,  0.5f, 0}, {0, 0, 1}, {1, 0}}
};

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();


    // TODO: Load Shader test
    Shader shader = GraphicsManager::Get()->CreateShader("./data/shaders/lineVert.hlsl",
                                                         "./data/shaders/lineFrag.hlsl");
    
    VertexBuffer quad = GraphicsManager::Get()->CreateVertexBuffer(gQuad, ARRAY_LENGTH(gQuad));

    while(PlatformManager::Get()->IsRunning()) {
        
        f32 millisecondsPerFrame = 16;
        f32 secondsPerFrame = millisecondsPerFrame / 1000.0f;

        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();
        
        GraphicsManager::Get()->ClearColorBuffer(0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearDepthStencilBuffer();

        // TODO: render the game
        GraphicsManager::Get()->DrawVertexBuffer(quad, shader);

        GraphicsManager::Get()->Present(1);
    }

    GraphicsManager::Get()->DestroyVertexBuffer(quad);

    GraphicsManager::Get()->DestroyShader(shader);

    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
