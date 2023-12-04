#include "level.h"
#include "geometry.h"
#include "steering_behaviors.h"

#include <float.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static Vertex gCubeVertices[] = {
        // positions          // texture Coords
   {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
   {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
   {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
   {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
   {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
   {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
                                                            
   {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
   {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
   {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
   {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
   {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}},
   {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
                                                            
   {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}},
   {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 1.0f}},
   {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}},
   {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}},
   {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 0.0f}},
   {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}},
                                                            
   {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}},
   {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 1.0f}},
   {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}},
   {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}},
   {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 0.0f}},
   {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}},
                                                            
   {{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f,  0.0f}, {0.0f, 1.0f}},
   {{ 0.5f, -0.5f, -0.5f}, {0.0f,-1.0f,  0.0f}, {1.0f, 1.0f}},
   {{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f,  0.0f}, {1.0f, 0.0f}},
   {{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f,  0.0f}, {1.0f, 0.0f}},
   {{-0.5f, -0.5f,  0.5f}, {0.0f,-1.0f,  0.0f}, {0.0f, 0.0f}},
   {{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f,  0.0f}, {0.0f, 1.0f}},
                                                            
   {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}},
   {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 1.0f}},
   {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}},
   {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}},
   {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 0.0f}},
   {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}}
};

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

static MapImporter::EntityFace gCubeFaces[] = {
    // bottom
    { {{ 0, -1,  0}, 32.0f}, {}, 0 },
    // top
    { {{ 0,  1,  0}, 32.0f}, {}, 0 },
    // left
    { {{-1,  0,  0}, 32.0f}, {}, 0 },
    // right
    { {{ 1,  0,  0}, 32.0f}, {}, 0 },
    // front
    { {{ 0,  0,  1}, 32.0f}, {}, 0 },
    // back
    { {{ 0,  0, -1}, 32.0f}, {}, 0 }
};

Vec3 *CreateCube() {
    Vec3 *cube = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(ARRAY_LENGTH(gCube) * sizeof(Vec3), 1);
    memcpy(cube, gCube, ARRAY_LENGTH(gCube) * sizeof(Vec3));
    return cube;
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
        mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer((SkinVertex *)mesh->vertices, mesh->numVertices, sizeof(SkinVertex));
        mesh->indexBuffer = GraphicsManager::Get()->CreateIndexBuffer(mesh->indices, mesh->numIndices);
    }
}

Entity *Level::AddEntity(const char *name, Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader) {
    Entity *entity = entitiesAllocator.Alloc();
    if(entitiesEnd == nullptr) entitiesEnd = entity;
    entity->Initialize(name, pos, rot, scale, model, shader, &map, entitiesEnd, &animationsSets[1]);
    entity->next = entities;
    if(entities != nullptr) {
        entities->prev = entity;
    }
    entities = entity;
    return entity;
}

Entity *Level::AddEntity(const char *name) {
    Entity *entity = entitiesAllocator.Alloc();
    if(entitiesEnd == nullptr) entitiesEnd = entity;
    entity->Initialize(name);
    entity->next = entities;
    if(entities != nullptr) {
        entities->prev = entity;
    }
    entities = entity;
    return entity;
}

Entity *Level::AddMovingPlatform(const char *name, Vec3 scale, Vec3 a, Vec3 b, Shader shader) {
    Entity *platform = AddEntity(name);

    TransformComponentDesc transformDesc = {};
    transformDesc.pos = a;
    transformDesc.rot = Vec3();
    transformDesc.scale = scale;
    platform->AddComponent<TransformComponent>(&transformDesc);

    // TODO: we dont need to load the model for each entity, reused it
    // load the mesh
    Mesh *mesh = (Mesh *)MemoryManager::Get()->AllocStaticMemory(sizeof(Mesh), 1); 
    mesh->texture = LoadTextureFromPath("cool.png");
    mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(gCubeVertices, ARRAY_LENGTH(gCubeVertices), sizeof(Vertex));
    mesh->indexBuffer = nullptr;

    // load the model and add the mesh to the model
    Model model = {};
    model.type = MODEL_TYPE_STATIC;
    model.numMeshes = 1;
    model.meshes = mesh;

    GraphicsComponentDesc graphCompDesc = {};
    graphCompDesc.model = model;
    graphCompDesc.shader = shader;
    platform->AddComponent<GraphicsComponent>(&graphCompDesc);

    CollisionComponentDesc colliderDesc = {};
    colliderDesc.type = COLLIDER_CONVEXHULL;
    colliderDesc.poly3D.convexHull.points = CreateCube();
    colliderDesc.poly3D.convexHull.count  = ARRAY_LENGTH(gCube);
    colliderDesc.poly3D.entity.faces = (MapImporter::EntityFace *)MemoryManager::Get()->AllocStaticMemory(sizeof(MapImporter::EntityFace) * 6, 1);
    memcpy(colliderDesc.poly3D.entity.faces, gCubeFaces, sizeof(MapImporter::EntityFace) * 6);
    colliderDesc.poly3D.entity.facesCount = 6;
    platform->AddComponent<CollisionComponent>(&colliderDesc);

    MovingPlatformComponentDesc movCompDesc = {};
    movCompDesc.a = a;
    movCompDesc.b = b;
    platform->AddComponent<MovingPlatformComponent>(&movCompDesc);

    return platform;
}

void Level::Initialize(char *mapFilePath, Shader statShader, Shader animShader) {
    entities = nullptr;
    entitiesEnd = nullptr;

    // NOTE Load Map ------------------------------------------------------------------------------------------
    MapImporter mapImporter;
    mapImporter.LoadMapFromFile(mapFilePath);
    MapImporter::VertexArray mapVertices = mapImporter.GetVertices();
    MapImporter::TextureArray mapTextures = mapImporter.GetTextures();
    
    map.covexHulls = mapImporter.GetConvexHulls();
    map.entities = mapImporter.GetEntities();
    map.vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count, sizeof(VertexMap));
    map.texture = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    map.scale = 1.0f/32.0f;

    // Load the BehaviorTree
    bhTree.Initialize();

    bhTree.AddNode<BehaviorSequence>(
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3(-16, 0, -6)),
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0, -6))
    );    

    // NOTE Load entities ------------------------------------------------------------------------------------
    ModelImporter modelImporter;
    
    // Load all level animations
    
    numAnimationsSets = 2;
    animationsSets = (AnimationClipSet *)MemoryManager::Get()->AllocStaticMemory(sizeof(AnimationClipSet) * numAnimationsSets, 1);
    
    AnimationImporter animationImporter;
    animationImporter.Read("./data/models/orc.twa");
    animationsSets[0].clips    = animationImporter.animations;
    animationsSets[0].numClips = animationImporter.numAnimations;
    animationsSets[0].skeleton = animationImporter.skeleton;

    animationImporter.Read("./data/models/hero.twa");
    animationsSets[1].clips    = animationImporter.animations;
    animationsSets[1].numClips = animationImporter.numAnimations;
    animationsSets[1].skeleton = animationImporter.skeleton;

    // Load Orc
    modelImporter.Read("./data/models/orc.twm");
    LoadModelToGpu(&modelImporter.model);
    orc = AddEntity("orc_1", Vec3(10, 4, 8), Vec3(), Vec3(1, 1, 1),
                    modelImporter.model, animShader);
    
    AIComponentDesc aiCompDesc = {};
    aiCompDesc.behavior = STEERING_BEHAVIOR_ARRIVE;
    aiCompDesc.timeToTarget = 0.75f;
    aiCompDesc.arrivalRadii = 2.0f;
    aiCompDesc.active = true;
    aiCompDesc.bhTree = &bhTree;
    orc->AddComponent<AIComponent>(&aiCompDesc);

    PlayerAnimationComponentDesc playerAnimCompuDesc = {};
    playerAnimCompuDesc.animationSet = &animationsSets[1];
    orc->AddComponent<PlayerAnimationComponent>(&playerAnimCompuDesc);

    // Load Orc1
    modelImporter.Read("./data/models/orc.twm");
    LoadModelToGpu(&modelImporter.model);
    orc1 = AddEntity("orc_2", Vec3(10, 4, 10), Vec3(), Vec3(1, 1, 1),
                     modelImporter.model, animShader);
    
    aiCompDesc = {};
    aiCompDesc.behavior = STEERING_BEHAVIOR_ARRIVE;
    aiCompDesc.timeToTarget = 0.75f;
    aiCompDesc.arrivalRadii = 4.0f;
    aiCompDesc.active = true;
    aiCompDesc.bhTree = nullptr;
    orc1->AddComponent<AIComponent>(&aiCompDesc);

    playerAnimCompuDesc = {};
    playerAnimCompuDesc.animationSet = &animationsSets[1];
    orc1->AddComponent<PlayerAnimationComponent>(&playerAnimCompuDesc);
    
    // Load Hero
    modelImporter.Read("./data/models/hero.twm");
    LoadModelToGpu(&modelImporter.model);
    hero = AddEntity("hero", Vec3(0, 30, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f),
                     modelImporter.model, animShader);

    InputComponentDesc inputCompDesc = {};
    inputCompDesc.input = PlatformManager::Get()->GetInput(); 
    inputCompDesc.camera = &camera; 
    hero->AddComponent<InputComponent>(&inputCompDesc);
    
    playerAnimCompuDesc = {};
    playerAnimCompuDesc.animationSet = &animationsSets[1];
    hero->AddComponent<PlayerAnimationComponent>(&playerAnimCompuDesc);

    // Load Horizontal Platform      scale             from             to
    platformHor  = AddMovingPlatform("mov_plat_1", Vec3(2, 0.5f, 2), Vec3(10,  3, -5), Vec3(10,  3, 5), statShader);
    platformVer0 = AddMovingPlatform("mov_plat_2", Vec3(2, 0.5f, 4), Vec3(14, 10,  0), Vec3(14,  3, 0), statShader);
    platformVer1 = AddMovingPlatform("mov_plat_3", Vec3(2, 0.5f, 2), Vec3(14, 10,  4), Vec3(14, 20, 4), statShader);

    camera.Initialize();

    TransformComponent *heroTransform = hero->GetComponent<TransformComponent>();
    TransformComponent *orcTransform = orc->GetComponent<TransformComponent>();
    gBlackBoard.target = &heroTransform->pos;
    
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

    TransformComponent *heroTransform = hero->GetComponent<TransformComponent>();
    camera.target = heroTransform->pos;
    camera.ProcessMovement(input, &map, dt);
    camera.SetViewMatrix();
}

void Level::Render(Shader mapShader) {
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(map.scale, map.scale, map.scale));
    GraphicsManager::Get()->BindTextureBuffer(map.texture);
    GraphicsManager::Get()->DrawVertexBuffer(map.vertexBuffer, mapShader);

    Entity *entity = entities;
    while(entity != nullptr) {
        entity->Render();
        entity = entity->next;
    }
}

