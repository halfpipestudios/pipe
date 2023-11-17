#include "level.h"
#include "geometry.h"

#include <float.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

Entity *Level::AddEntity(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations) {
    Entity *entity = entitiesAllocator.Alloc();
    entity->Initialize(pos, rot, scale, model, animations, numAnimations, &map);
    entity->next = entities;
    entities = entity;
    return entity;
}

void Level::Initialize(char *mapFilePath) {
    entities = nullptr;

    // NOTE Load Map ------------------------------------------------------------------------------------------
    MapImporter mapImporter;
    mapImporter.LoadMapFromFile(mapFilePath);
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
    animationImporter.Read("./data/models/orc.twa");
    LoadModelToGpu(&modelImporter.model);
    orc = AddEntity(Vec3(6, 2, 0), Vec3(), Vec3(1, 1, 1),
                    modelImporter.model, animationImporter.animations, 
                    animationImporter.numAnimations);


    modelImporter.Read("./data/models/hero.twm");
    animationImporter.Read("./data/models/hero.twa");
    LoadModelToGpu(&modelImporter.model);
    hero = AddEntity(Vec3(0, 30, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f),
                     modelImporter.model, animationImporter.animations, 
                     animationImporter.numAnimations);

    StateMachineComponentDesc stmCompDesc = {};
    stmCompDesc.camera = &camera;
    hero->AddComponent<StateMachineComponent>(&stmCompDesc);


    camera.Initialize();
}

void Level::Terminate() {
    GraphicsManager::Get()->DestroyTextureBuffer(map.texture);
    GraphicsManager::Get()->DestroyVertexBuffer(map.vertexBuffer);
    
    Entity *entity = entities;
    while(entity != nullptr) {
        
        Entity *toFree = entity;
        entity = entity->next; 

        toFree->Terminate();
        entitiesAllocator.Free(toFree);
    }

}

void Level::Update(f32 dt) {

    Input *input = PlatformManager::Get()->GetInput();

    Entity *entity = entities;
    while(entity != nullptr) {
        entity->Update(&map, dt);
        entity = entity->next;
    }

    GraphicsComponent *heroGraphics = hero->GetComponent<GraphicsComponent>();
    camera.target = heroGraphics->transform.pos;
    camera.ProcessMovement(input, &map, dt);
    camera.SetViewMatrix();
}

void Level::Render(Shader statShader, Shader animShader) {
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(map.scale, map.scale, map.scale));
    GraphicsManager::Get()->BindTextureBuffer(map.texture);
    GraphicsManager::Get()->DrawVertexBuffer(map.vertexBuffer, statShader);

    Entity *entity = entities;
    while(entity != nullptr) {
        entity->Render(animShader);
        entity = entity->next;
    }
}

