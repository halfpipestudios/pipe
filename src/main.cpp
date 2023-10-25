#include <stdio.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include "map_importer.h"

struct Camera {

    Camera(Vec3 pos_, f32 speed_)
        : pos(pos_), speed(speed_) {}

    Vec3 pos;
    Vec2 rot;
    Vec3 front;
    Vec3 right;
    Vec3 up;
    f32 speed;

    void ProcessMovement(Input *input);
};

void Camera::ProcessMovement(Input *input)
{
    front = {0, 0, 1};
    front = Mat4::TransformVector(Mat4::RotateX(rot.x), front);
    front = Mat4::TransformVector(Mat4::RotateY(rot.y), front);
    front.Normalize();

    right = Vec3(0, 1, 0).Cross(front).Normalized();
    up = front.Cross(right).Normalized();

    Vec3 acc = {};
    Vec3 worldFront = right.Cross(Vec3(0, 1, 0)).Normalized();

    if(input->KeyIsPress(KEY_W)) {
        acc += worldFront;
    }
    if(input->KeyIsPress(KEY_S)) {
        acc -= worldFront;
    }
    if(input->KeyIsPress(KEY_A)) {
        acc -= right;
    }
    if(input->KeyIsPress(KEY_D)) {
        acc += right;
    }
    if(input->KeyIsPress(KEY_R)) {
        acc += Vec3(0, 1, 0);
    }
    if(input->KeyIsPress(KEY_F)) {
        acc -= Vec3(0, 1, 0);
    }

    if(input->KeyIsPress(KEY_LEFT)) {
        rot.y += speed * 0.25f;
    }
    if(input->KeyIsPress(KEY_RIGHT)) {
        rot.y -= speed * 0.25f;
    }
    if(input->KeyIsPress(KEY_UP)) {
        rot.x += speed * 0.25f;
    }
    if(input->KeyIsPress(KEY_DOWN)) {
        rot.x -= speed * 0.25f;
    }

    if (rot.x >  (89.0f/180.0f) * PI)
        rot.x =  (89.0f/180.0f) * PI;
    if (rot.x < -(89.0f/180.0f) * PI)
        rot.x = -(89.0f/180.0f) * PI;

    if(acc.LenSq() > 0.0f) acc.Normalize();

    acc *= speed;

    pos += acc;

    Vec3 up  = Vec3(0, 1,  0);
    GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos, pos + front, Vec3(0, 1, 0)));
}

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();


    // TODO: Load Shader test
    Shader shader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/texVert.hlsl",
                                                                  "./data/shaders/texFrag.hlsl");
    
    // Test code to load the level .map file
    MapLoader loader;
    loader.LoadMapFromFile("./data/maps/test.map");

    VertexArray mapVertices = loader.GetVertices();
    TextureArray mapTextures = loader.GetTextures();

    VertexBuffer  mapVBO = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count);
    TextureBuffer mapSRV = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);


    // Set Matrices
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                80, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));

    f32 scale = 1.0f/32.0f;
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));

    Camera camera(Vec3(0, 2, -5), 0.1f); 


    while(PlatformManager::Get()->IsRunning()) {
        
        f32 millisecondsPerFrame = 16;
        f32 secondsPerFrame = millisecondsPerFrame / 1000.0f;

        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();

        // Temporal function ...
        camera.ProcessMovement(input);
        
        GraphicsManager::Get()->ClearColorBuffer(0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearDepthStencilBuffer();

        // TODO: render the game
        GraphicsManager::Get()->BindTextureBuffer(mapSRV);
        GraphicsManager::Get()->DrawVertexBuffer(mapVBO, shader);

        GraphicsManager::Get()->Present(1);
    }

    MemoryManager::Get()->ClearStaticMemory();

    GraphicsManager::Get()->DestroyTextureBuffer(mapSRV);

    GraphicsManager::Get()->DestroyVertexBuffer(mapVBO);

    GraphicsManager::Get()->DestroyShader(shader);

    GraphicsManager::Get()->Terminate();
    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
