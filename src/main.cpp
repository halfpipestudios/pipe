#include <stdio.h>
#include <math.h>

#include "platform_manager.h"
#include "memory_manager.h"
#include "graphics_manager.h"

#include "map_importer.h"
#include "model_importer.h"

#include "gjk_collision.h"

#include "animation.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Camera {

    Camera(Vec3 pos_, f32 speed_)
        : pos(pos_), speed(speed_),
          vel(Vec3()), acc(Vec3()),
          grounded(false), maxDist(5), dist(5) {}

    Vec3 pos;
    Vec3 lastPos;
    Vec2 rot;
    Vec3 front;
    Vec3 right;
    Vec3 up;
    f32 speed;

    f32 dist;
    f32 maxDist;

    Vec3 vel;
    Vec3 acc;
    bool grounded;

    void ProcessMovement(Input *input, f32 deltaTime);
    void SetViewMatrix();
};

void Camera::SetViewMatrix() {
    Vec3 up  = Vec3(0, 1,  0);
    GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos - front * dist, pos, Vec3(0, 1, 0)));
}

void Camera::ProcessMovement(Input *input, f32 deltaTime)
{
    front = {0, 0, 1};
    front = Mat4::TransformVector(Mat4::RotateX(rot.x), front);
    front = Mat4::TransformVector(Mat4::RotateY(rot.y), front);
    front.Normalize();

    right = Vec3(0, 1, 0).Cross(front).Normalized();
    up = front.Cross(right).Normalized();

    Vec3 worldFront = right.Cross(Vec3(0, 1, 0)).Normalized();

    acc = {};
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

    f32 zoomSpeed = 0.25f;
    if(input->KeyIsPress(KEY_F)) {
        maxDist = MIN(maxDist + zoomSpeed, 15.0f);

    }
    if(input->KeyIsPress(KEY_R)) {
        maxDist = MAX(maxDist - zoomSpeed, 1.0f);
    }

    if(input->KeyJustPress(KEY_SPACE) && grounded) {
        Vec3 jumpForce = Vec3(0, 40, 0);
        vel += jumpForce;
    }

    if(input->KeyIsPress(KEY_LEFT)) {
        rot.y += 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_RIGHT)) {
        rot.y -= 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_UP)) {
        rot.x += 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_DOWN)) {
        rot.x -= 2.5f * deltaTime;
    }

    if (rot.x >  (89.0f/180.0f) * PI)
        rot.x =  (89.0f/180.0f) * PI;
    if (rot.x < -(89.0f/180.0f) * PI)
        rot.x = -(89.0f/180.0f) * PI;

    if(acc.LenSq() > 0.0f) acc.Normalize();

    acc *= speed;

    if(!grounded)
        acc += Vec3(0, -9.8 * 5, 0);

    vel += acc * deltaTime;

    f32 dammping = powf(0.001f, deltaTime);
    vel = vel * dammping;

    lastPos = pos;
    pos += vel * deltaTime;
}

struct Segment {
    Vec3 a;
    Vec3 b;

    bool HitEntity(Entity *entity, f32 *tOut);
};

bool Segment::HitEntity(Entity *entity, f32 *tOut) {
    Vec3 d = b - a;
    // Set initial interval to being the whole segment. For a ray, tlast should be
    // sety to FLT_MAX. For a line tfirst should be set to - FLT_MAX
    f32 tFirst = 0.0f;
    f32 tLast = 1.0f;
    // intersect segment agains each plane
    for(i32 i = 0; i < entity->facesCount; i++)
    {
        Plane p = entity->faces[i].plane;

        f32 denom = p.n.Dot(d);
        f32 dot = p.n.Dot(a);
        f32 dist = dot - (p.d/32.0f);
        // test if segment runs parallel to tha plane
        if(denom == 0.0f)
        {
            // If so, return "no intersection" if segemnt lies outside the plane
            if(dist > 0.0f) return 0;
        }
        else
        {
            f32 t = -(dist / denom);
            if(denom < 0.0f)
            {
                // when entering halfspace, update tfirst if t is larger
                if(t > tFirst) tFirst = t;
            }
            else
            {
                // when exiting halfspace, update tLast if t is smaller
                if(t < tLast) tLast = t;
            }

            if(tFirst > tLast) return 0;
        }
    }
    *tOut = tFirst;
    return 1;
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

static TextureBuffer LoadTextureFromPath(char *path) {
    static char diffuse_material_path_cstr[4096];
    sprintf(diffuse_material_path_cstr, "%s%s", "./data/textures/", path); 
    
    stbi_set_flip_vertically_on_load(true);
    i32 w, h, n;
    u32 *bitmap = (u32 *)stbi_load(diffuse_material_path_cstr, &w, &h, &n, 4);
    
    Texture texture = {bitmap, w, h};
    TextureBuffer textureBuffer = GraphicsManager::Get()->CreateTextureBuffer(&texture, 1);
    
    stbi_image_free(bitmap);

    printf("Texture: %s loaded\n", diffuse_material_path_cstr);

    return textureBuffer;
}

static void LoadModelToGpu(Model *model) {
    ASSERT(model->type == MODEL_TYPE_ANIMATED);
    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex; 
        mesh->texture = LoadTextureFromPath(mesh->material);
        mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer((SkinVertex *)mesh->vertices, mesh->numVertices);
        mesh->indexBuffer = GraphicsManager::Get()->CreateIndexBuffer(mesh->indices, mesh->numIndices);
    }
}

int main() {

    PlatformManager::Get()->Initialize();
    MemoryManager::Get()->Initialize();
    GraphicsManager::Get()->Initialize();

    Input *input = PlatformManager::Get()->GetInput();

    // TODO: Load Shader test
    Shader shader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/texVert.hlsl",
                                                                  "./data/shaders/texFrag.hlsl");
    
    Shader animShader = GraphicsManager::Get()->CreateShaderSkinVertex("./data/shaders/animVert.hlsl",
                                                                       "./data/shaders/texFrag.hlsl");
    // Test code to load the level .map file
    MapLoader loader;
    loader.LoadMapFromFile("./data/maps/test.map");
    VertexArray mapVertices = loader.GetVertices();
    TextureArray mapTextures = loader.GetTextures();
    ConvexHullArray mapCovexHulls = loader.GetConvexHulls();
    EntityArray mapEntityArray = loader.GetEntities();

    VertexBuffer  mapVBO = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count);
    TextureBuffer mapSRV = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    
    // Test code to load model and animation file
    ModelImporter modelImporter;
    modelImporter.Read("./data/models/orc.twm");
    LoadModelToGpu(&modelImporter.model);

    AnimationImporter animationImporter;
    animationImporter.Read("./data/models/orc.twa");
    
    AnimationSet animation;
    animation.Initialize(animationImporter.animations, animationImporter.numAnimations);
    
    animation.Play("idle", 1, true);

    // Test other model
    ModelImporter heroImporter;
    heroImporter.Read("./data/models/hero.twm");
    LoadModelToGpu(&heroImporter.model);

    AnimationImporter heroAnimationImporter;
    heroAnimationImporter.Read("./data/models/hero.twa");
    
    AnimationSet heroAnimation;
    heroAnimation.Initialize(heroAnimationImporter.animations, heroAnimationImporter.numAnimations);
    
    heroAnimation.Play("idle", 1, true);
    heroAnimation.Play("walking", 1, true);

    // Temporal identities matricies
    static Mat4 identity[100];
    for(u32 i = 0; i < 100; ++i) {
        identity[i] = Mat4();
    }

    // Set Matrices
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));

    f32 scale = 1.0f/32.0f;
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));

    Camera camera(Vec3(0, 30, 0), 40.0f); 

    Cylinder cylinder;
    cylinder.c = Vec3(4, 2, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;

    Vec3 *cubeA = CreateCube();
    Vec3 *cubeB = CreateCube();

    ConvexHull aHull;
    aHull.points = cubeA;
    aHull.count = ARRAY_LENGTH(gCube);
    ConvexHull bHull;
    bHull.points = cubeB;
    bHull.count = ARRAY_LENGTH(gCube);

    GJK gjk;

    f64 lastTimer = PlatformManager::Get()->GetTimeInSeconds();
    u32 FPS = 0;
    while(PlatformManager::Get()->IsRunning()) {

        f64 currentTime = PlatformManager::Get()->GetTimeInSeconds();
        f64 deltaTime = currentTime - lastTimer;

        lastTimer = currentTime;

        static f32 time = 0;
        time += deltaTime;

 
        f32 millisecondsPerFrame = 16;
        f32 secondsPerFrame = millisecondsPerFrame / 1000.0f;

        PlatformManager::Get()->PollEvents();
        MemoryManager::Get()->ClearFrameMemory();

        // Temporal function ...
        camera.ProcessMovement(input, deltaTime);
        
        cylinder.c = camera.pos;

        TransformCube(cubeA, Vec3(10, 3, sinf(time)), 0.0f);
        TransformCube(cubeB, Vec3(12, 3, 1), 0.0f);
        
        u32 colorA = 0xFF0000FF;
        u32 colorB = 0xFF0000FF;
        u32 colorC = 0xFF0000FF;

        Segment playerSegment;
        playerSegment.a =  camera.lastPos;
        playerSegment.b = camera.pos;
        Vec3 cameraDir = camera.pos - camera.lastPos;
        f32 tMin = FLT_MAX;
        for(i32 i = 0; i < mapEntityArray.count; ++i) {
            Entity *entity = &mapEntityArray.data[i];
            f32 t = -1.0f;
            if(playerSegment.HitEntity(entity, &t)) {
               if(t < tMin) {
                    tMin = t;
                }
            }
        }

        if(tMin >= 0.0f && tMin <= 1.0f) {
            camera.pos = camera.lastPos + (camera.pos - camera.lastPos) * (tMin*0.8);
            cylinder.c = camera.pos;
        }

        CollisionData collisionData = gjk.Intersect(&aHull, &cylinder);
        if(collisionData.hasCollision) {
            colorA = 0xFFFF0000;
            colorC = 0xFFFF0000;
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            cylinder.c += normal * penetration; 
            camera.pos = cylinder.c;
            camera.vel -= camera.vel.Dot(normal)*normal;
        }

        collisionData = gjk.Intersect(&bHull, &cylinder);
        if(collisionData.hasCollision) {
            colorB = 0xFFFF0000;
            colorC = 0xFFFF0000;
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            cylinder.c += normal * penetration; 
            camera.pos = cylinder.c;
            camera.vel -= camera.vel.Dot(normal)*normal;
        }

        for(i32 i = 0; i < mapCovexHulls.count; ++i) {
            ConvexHull *hull = &mapCovexHulls.data[i];
            CollisionData collisionData = gjk.Intersect(hull, &cylinder);
            if(collisionData.hasCollision) {
                colorC = 0xFFFF0000;
                f32 penetration = collisionData.penetration;
                Vec3 normal = collisionData.normal;
                cylinder.c += normal * penetration; 
                camera.pos = cylinder.c;
                camera.vel -= camera.vel.Dot(normal)*normal;
            }
        }

        Segment cameraSegment;
        cameraSegment.a = camera.pos;
        cameraSegment.b = (camera.pos - camera.front * camera.maxDist);

        Segment groundSegment;
        groundSegment.a = cylinder.c;
        groundSegment.b = groundSegment.a + Vec3(0, -(cylinder.n + 0.001), 0);
        camera.grounded = false;

        tMin = FLT_MAX;
        for(i32 i = 0; i < mapEntityArray.count; ++i) {
            Entity *entity = &mapEntityArray.data[i];
            f32 t = -1.0f;
            if(groundSegment.HitEntity(entity, &t)) {
                camera.grounded = true;
            }
            if(cameraSegment.HitEntity(entity, &t)) {
                if(t < tMin) {
                    tMin = t;
                }
            }
        }

        camera.dist = camera.maxDist;
        if(tMin > 0.0f && tMin <= 1.0f) {
            camera.dist = MIN((camera.maxDist-0.1f) * tMin, camera.maxDist);
        }

        camera.SetViewMatrix();
        
        Vec2 cameraVel = Vec2(camera.vel.x, camera.vel.z);
        heroAnimation.UpdateWeight("walking", CLAMP(cameraVel.Len()*0.25f, 0, 1));

        Mat4 *finalTransformMatricesOut = nullptr;
        u32 numFinaltrasformMatricesOut = 0;
        heroAnimation.Update(deltaTime, &finalTransformMatricesOut, &numFinaltrasformMatricesOut);
        GraphicsManager::Get()->SetAnimMatrices(finalTransformMatricesOut, numFinaltrasformMatricesOut);

        // printf("Remaining memory storage %lld\n", MemoryManager::Get()->RemainingMemorySotrage());

        // TODO: render the game
        GraphicsManager::Get()->ClearColorBuffer(0.5f, 0.0f, 1.0f);
        GraphicsManager::Get()->ClearDepthStencilBuffer();

        // draw the level
        GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));
        GraphicsManager::Get()->BindTextureBuffer(mapSRV);
        GraphicsManager::Get()->DrawVertexBuffer(mapVBO, shader);

        // NOTE: Draw player
        GraphicsManager::Get()->SetWorldMatrix(Mat4::Translate(camera.pos - Vec3(0, 0.75f, 0)) *
                                               Mat4::RotateY(camera.rot.y) *
                                               Mat4::Scale(0.8f, 0.8f, 0.8f));
        for(u32 meshIndex = 0; meshIndex < heroImporter.model.numMeshes; ++meshIndex) {
            Mesh *mesh = heroImporter.model.meshes + meshIndex;
            GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, animShader);
        }
        
       //GraphicsManager::Get()->SetAnimMatrices(identity, 100);
       finalTransformMatricesOut = nullptr;
       numFinaltrasformMatricesOut = 0;
       animation.Update(deltaTime, &finalTransformMatricesOut, &numFinaltrasformMatricesOut);
       GraphicsManager::Get()->SetAnimMatrices(finalTransformMatricesOut, numFinaltrasformMatricesOut);

        // NOTE: Draw NPC
        GraphicsManager::Get()->SetWorldMatrix(Mat4::Translate(Vec3(6, 2, 0)) * Mat4::RotateY(TO_RAD(180.0f)) * Mat4::Scale(1,1,1));
        for(u32 meshIndex = 0; meshIndex < modelImporter.model.numMeshes; ++meshIndex) {
            Mesh *mesh = modelImporter.model.meshes + meshIndex;
            GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, animShader);
        }

        // draw the debug geometry
        DrawCube(cubeA, colorA);
        DrawCube(cubeB, colorB);
        DrawCylinder(cylinder, colorC);

        u32 rayColor = camera.grounded ? 0xFFFF0000 : 0xFF00FF00;
        GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, rayColor);
        GraphicsManager::Get()->DrawLine(cameraSegment.a, cameraSegment.b, 0xFFFF00FF);

        GraphicsManager::Get()->Present(1);
    }

    animation.Terminate();

    GraphicsManager::Get()->DestroyTextureBuffer(mapSRV);

    GraphicsManager::Get()->DestroyVertexBuffer(mapVBO);

    GraphicsManager::Get()->DestroyShader(shader);
    GraphicsManager::Get()->DestroyShader(animShader);

    GraphicsManager::Get()->Terminate();

    MemoryManager::Get()->ClearStaticMemory();

    MemoryManager::Get()->Terminate();
    PlatformManager::Get()->Terminate();

    printf("Pipe Engine Terminate!\n");

    return 0;
}
