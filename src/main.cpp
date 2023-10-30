#include <stdio.h>
#include <math.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include "map_importer.h"
#include "gjk_collision.h"

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

static Vec3 gCube[] = {

    // bottom
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    // top
    Vec3(-0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f, -0.5f),
    Vec3(-0.5, 0.5f, -0.5f),
    // left
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3(-0.5,  0.5f,  0.5f),
    // right
    Vec3(0.5, -0.5f,  0.5f),
    Vec3(0.5, -0.5f, -0.5f),
    Vec3(0.5,  0.5f, -0.5f),
    Vec3(0.5,  0.5f,  0.5f),
    // front
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3( 0.5,  0.5f, -0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    // back
    Vec3(-0.5, -0.5f, 0.5f),
    Vec3(-0.5,  0.5f, 0.5f),
    Vec3( 0.5,  0.5f, 0.5f),
    Vec3( 0.5, -0.5f, 0.5f)

};

Vec3 *CreateCube() {
    Vec3 *cube = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(ARRAY_LENGTH(gCube) * sizeof(Vec3), 1);
    memcpy(cube, gCube, ARRAY_LENGTH(gCube) * sizeof(Vec3));
    return cube;
}

void TransformCube(Vec3 *cube,  Vec3 p, f32 angle) {
    for(i32 i = 0; i < 24; ++i) {
        cube[i] = Mat4::TransformPoint(Mat4::Translate(p.x, p.y, p.z) * Mat4::RotateX(angle), gCube[i]);
    }
}

void DrawCube(Vec3 *cube, u32 color) {
    for(i32 faceIndex = 0; faceIndex < 6; ++faceIndex) {
        
        Vec3 *vertices = &cube[faceIndex * 4];
        
        for(i32 i = 0; i < 4; ++i) {
            Vec3 a = vertices[i];
            Vec3 b = vertices[(i + 1) % 4];

            GraphicsManager::Get()->DrawLine(a, b, color);
        }
    }
}

void DrawCylinder(Cylinder cylinder, u32 color) {
    Vec3 vertices[40] = {};

    // TODO: add rotatons
    // Top face
    f32 increment = (2.0f * PI) / 20;
    f32 angle = 0.0f;
    for(i32 i = 0; i < 20; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c + cylinder.u * cylinder.n;
        angle += increment;
    }

    // Bottom face
    angle = 0.0f;
    for(i32 i = 20; i < 40; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c - cylinder.u * cylinder.n;
        angle += increment;
    }

    // Rendering code
    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[i];
        Vec3 b = vertices[(i + 1) % 20];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[20 + i];
        Vec3 b = vertices[20 + (i + 1) % 20];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[i];
        Vec3 b = vertices[20 + i];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

}

Vec3 *CreateCylinderVertices(Cylinder cylinder) {
    Vec3 *vertices = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(40 * sizeof(Vec3), 1);

    // Top face
    f32 increment = (2.0f * PI) / 20;
    f32 angle = 0.0f;
    for(i32 i = 0; i < 20; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c + cylinder.u * cylinder.n;
        angle += increment;
    }

    // Bottom face
    angle = 0.0f;
    for(i32 i = 20; i < 40; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c - cylinder.u * cylinder.n;
        angle += increment;
    }
    return vertices;
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
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));

    f32 scale = 1.0f/32.0f;
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));

    Camera camera(Vec3(0, 2, 0), 0.2f); 

    Cylinder cylinder;
    cylinder.c = Vec3(4, 2, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.5f;
    cylinder.n = 0.5f;

    Vec3 *cubeA = CreateCube();
    Vec3 *cubeB = CreateCube();

    ConvexHull aHull;
    aHull.points = cubeA;
    aHull.count = ARRAY_LENGTH(gCube);
    ConvexHull bHull;
    bHull.points = cubeB;
    bHull.count = ARRAY_LENGTH(gCube);

    GJK gjk;

    while(PlatformManager::Get()->IsRunning()) {

        static f32 time = 0;
        time += 0.01f;
        
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


        if(input->KeyIsPress(KEY_SHIFT)) {
            cylinder.c += Vec3(0, 0.05f, 0);
        }
        if(input->KeyIsPress(KEY_CONTROL)) {
            cylinder.c += Vec3(0, -0.05f, 0);
        }

        if(input->KeyIsPress(KEY_I)) {
            cylinder.c += Vec3(0, 0,  0.05f);
        }
        if(input->KeyIsPress(KEY_K)) {
            cylinder.c += Vec3(0, 0, -0.05f);
        }

        if(input->KeyIsPress(KEY_J)) {
            cylinder.c += Vec3(-0.05f, 0, 0);
        }
        if(input->KeyIsPress(KEY_L)) {
            cylinder.c += Vec3( 0.05f, 0, 0);
        }

        TransformCube(cubeA, Vec3( 0, 2, 0), 0.0f);
        TransformCube(cubeB, Vec3(-2, 2, 0), 0.0f);

        
        u32 colorA = 0xFF0000FF;
        u32 colorB = 0xFF0000FF;
        u32 colorC = 0xFF0000FF;

        CollisionData collisionData = gjk.Intersect(&aHull, &cylinder);
        if(collisionData.hasCollision) {
            colorA = 0xFFFF0000;
            colorC = 0xFFFF0000;
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            cylinder.c += normal * penetration; 
        }

        collisionData = gjk.Intersect(&bHull, &cylinder);
        if(collisionData.hasCollision) {
            colorB = 0xFFFF0000;
            colorC = 0xFFFF0000;
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            cylinder.c += normal * penetration; 
        }

        DrawCube(cubeA, colorA);
        DrawCube(cubeB, colorB);
        DrawCylinder(cylinder, colorC);

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
