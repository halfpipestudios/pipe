#include <stdio.h>
#include <float.h>

#include "stb_image.h"

#include "game.h"
#include "platform_manager.h" 
#include "geometry.h"

static void DrawCylinder(Cylinder cylinder, u32 color) {
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

void Game::Initialize() {

    // Set Matrices
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(
                60, 
                (f32)PlatformManager::Get()->GetWindow()->GetWidth() /
                (f32)PlatformManager::Get()->GetWindow()->GetHeight(),
                0.01f, 1000.0f));

    GraphicsManager::Get()->SetWorldMatrix(Mat4());

    statShader = GraphicsManager::Get()->CreateShaderVertexMap("./data/shaders/texVert.hlsl",
                                                               "./data/shaders/texFrag.hlsl");
    animShader = GraphicsManager::Get()->CreateShaderSkinVertex("./data/shaders/animVert.hlsl",
                                                                "./data/shaders/texFrag.hlsl");

    level.Initialize("./data/maps/test.map");
    #if 0
// NOTE Load Map ------------------------------------------------------------------------------------------
    MapImporter mapImporter;
    mapImporter.LoadMapFromFile("./data/maps/test.map");
    MapImporter::VertexArray mapVertices = mapImporter.GetVertices();
    MapImporter::TextureArray mapTextures = mapImporter.GetTextures();
    
    map.covexHulls = mapImporter.GetConvexHulls();
    map.entities = mapImporter.GetEntities();
    map.vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count);
    map.texture = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    map.scale = 1.0f/32.0f;


// NOTE Load entities ------------------------------------------------------------------------------------
    ModelImporter modelImporter;
    AnimationImporter animationImporter;

    modelImporter.Read("./data/models/orc.twm");
    orc = modelImporter.model;
    LoadModelToGpu(&orc);
    animationImporter.Read("./data/models/orc.twa");
    orcAnimation.Initialize(animationImporter.animations,
                            animationImporter.numAnimations);
    orcCollider.c = Vec3(6, 2.75f, 0);
    orcCollider.u = Vec3(0, 1, 0);
    orcCollider.radii = 0.3f;
    orcCollider.n = 0.75f;
    
    modelImporter.Read("./data/models/hero.twm");
    hero = modelImporter.model;
    LoadModelToGpu(&hero);
    animationImporter.Read("./data/models/hero.twa");
    heroAnimation.Initialize(animationImporter.animations,
                             animationImporter.numAnimations);
    heroCollider.c = Vec3(camera.pos);
    heroCollider.u = Vec3(0, 1, 0);
    heroCollider.radii = 0.3f;
    heroCollider.n = 0.75f;

    heroAnimation.Play("idle", 1, true);
    heroAnimation.Play("walking", 1, true);

    orcAnimation.Play("idle", 1, true);
    #endif


}

void Game::Terminate() {
#if 0
    GraphicsManager::Get()->DestroyTextureBuffer(map.texture);
    GraphicsManager::Get()->DestroyVertexBuffer(map.vertexBuffer);

    orcAnimation.Terminate();
    heroAnimation.Terminate();
#endif
    
    level.Terminate();

    GraphicsManager::Get()->DestroyShader(statShader);
    GraphicsManager::Get()->DestroyShader(animShader);

}

void Game::Update(f32 dt) {

    // NOTE: process movement and collision
    level.Update(dt);

#if 0
    heroCollider.c = camera.pos;

    colorC = 0xFF0000FF;

    Segment playerSegment;
    playerSegment.a =  camera.lastPos;
    playerSegment.b = camera.pos;
    Vec3 cameraDir = camera.pos - camera.lastPos;
    f32 tMin = FLT_MAX;
    for(i32 i = 0; i < map.entities.count; ++i) {
        MapImporter::Entity *entity = &map.entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(entity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }

    if(tMin >= 0.0f && tMin <= 1.0f) {
        camera.pos = camera.lastPos + (camera.pos - camera.lastPos) * (tMin*0.8);
        heroCollider.c = camera.pos;
    }

    GJK gjk;
    for(i32 i = 0; i < map.covexHulls.count; ++i) {
        ConvexHull *hull = &map.covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &heroCollider);
        if(collisionData.hasCollision) {
            colorC = 0xFFFF0000;
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            heroCollider.c += normal * penetration; 
            camera.pos = heroCollider.c;
            camera.vel -= camera.vel.Dot(normal)*normal;
        }
    }

    Segment cameraSegment;
    cameraSegment.a = camera.pos;
    cameraSegment.b = (camera.pos - camera.front * camera.maxDist);

    Segment groundSegment;
    groundSegment.a = heroCollider.c;
    groundSegment.b = groundSegment.a + Vec3(0, -(heroCollider.n + 0.001), 0);
    camera.grounded = false;

    tMin = FLT_MAX;
    for(i32 i = 0; i < map.entities.count; ++i) {
        MapImporter::Entity *entity = &map.entities.data[i];
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

// NOTE: update animations
    Vec2 cameraVel = Vec2(camera.vel.x, camera.vel.z);
    heroAnimation.UpdateWeight("walking", CLAMP(cameraVel.Len()*0.25f, 0, 1));

    heroAnimation.Update(dt, &heroFinalTransformMatrices, &heroNumFinalTrasformMatrices);
    orcAnimation.Update(dt, &orcFinalTransformMatrices, &orcNumFinalTrasformMatrices);
#endif
}


void Game::Render() {
    
    level.Render(statShader, animShader);

#if 0
    // draw the level
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(map.scale, map.scale, map.scale));
    GraphicsManager::Get()->BindTextureBuffer(map.texture);
    GraphicsManager::Get()->DrawVertexBuffer(map.vertexBuffer, statShader);

   // NOTE: Draw Hero
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Translate(camera.pos - Vec3(0, 0.75f, 0)) *
                                           Mat4::RotateY(camera.rot.y) *
                                           Mat4::Scale(0.8f, 0.8f, 0.8f));

    GraphicsManager::Get()->SetAnimMatrices(heroFinalTransformMatrices, heroNumFinalTrasformMatrices);
    for(u32 meshIndex = 0; meshIndex < hero.numMeshes; ++meshIndex) {
        Mesh *mesh = hero.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, animShader);
    }

    // NOTE: Draw Orc
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Translate(Vec3(6, 2, 0)) * Mat4::RotateY(TO_RAD(180.0f)) * Mat4::Scale(1,1,1));
    GraphicsManager::Get()->SetAnimMatrices(orcFinalTransformMatrices, orcNumFinalTrasformMatrices);
    for(u32 meshIndex = 0; meshIndex < orc.numMeshes; ++meshIndex) {
        Mesh *mesh = orc.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, animShader);
    }
    
    // NOTE: Draw debug geometry
    DrawCylinder(heroCollider, colorC);
    DrawCylinder(orcCollider, 0xFF00FF00);
#endif
}


// -----------------------------------------------------------------------
void Game::FixUpdate(f32 dt) {
    // TODO: ...
}

void Game::PostUpdate(f32 t) {
    // TODO: ...
}
