#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include "map_importer.h"

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

    MapLoader loader;
    EntityArray array = loader.LoadMapFromFile("./data/maps/test.map");

    VertexArray mapVertices = loader.VertexArrayCreateFromEntityArray(array);

    VertexBuffer map = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count);

    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));

    Vec3 pos = Vec3(0, 2, -5);
    Vec3 tar = Vec3(0, 2,  0);
    Vec3 up  = Vec3(0, 1,  0);
    GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos, tar, up));

    f32 scale = 1.0f/128.0f;
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));


    while(PlatformManager::Get()->IsRunning()) {
        
        f32 millisecondsPerFrame = 16;
        f32 secondsPerFrame = millisecondsPerFrame / 1000.0f;

        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();
        
        GraphicsManager::Get()->ClearColorBuffer(0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearDepthStencilBuffer();

        // TODO: render the game
        GraphicsManager::Get()->DrawVertexBuffer(map, shader);

        GraphicsManager::Get()->Present(1);
    }

    MemoryManager::Get()->ClearStaticMemory();

    GraphicsManager::Get()->DestroyVertexBuffer(quad);
    GraphicsManager::Get()->DestroyVertexBuffer(map);

    GraphicsManager::Get()->DestroyShader(shader);

    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
