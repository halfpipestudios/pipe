#include "level.h"
#include "geometry.h"
#include "steering_behaviors.h"

#include <float.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "cmp_factory.h"

#include "mgr/texture_manager.h"
#include "mgr/model_manager.h"
#include "mgr/animation_manager.h"

#include "globals.h"

void Map::Initialize(char *filename) {

    MapImporter mapImporter;
    mapImporter.LoadMapFromFile(filename);
    MapImporter::VertexArray mapVertices = mapImporter.GetVertices();
    MapImporter::TextureArray mapTextures = mapImporter.GetTextures();
    
    covexHulls = mapImporter.GetConvexHulls();
    entities = mapImporter.GetEntities();
    vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count, sizeof(VertexMap));
    texture = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    scale = 1.0f/32.0f;
}

void Map::Terminate() {
    GraphicsManager::Get()->DestroyTextureBuffer(texture);
    GraphicsManager::Get()->DestroyVertexBuffer(vertexBuffer);
}


void Map::Render() {
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));
    GraphicsManager::Get()->BindTextureBuffer(texture);
    
    VShader vShader = *VShaderManager::Get()->Dereference(VShaderManager::Get()->GetAsset("mapVert.hlsl"));
    FShader fShader = *FShaderManager::Get()->Dereference(FShaderManager::Get()->GetAsset("mapFrag.hlsl"));

    GraphicsManager::Get()->DrawVertexBuffer(vertexBuffer, vShader, fShader);
}


static SlotmapKey CreateHero(Camera *camera) {

    Input *input = PlatformManager::Get()->GetInput();

    SlotmapKey heroKey = EntityManager::Get()->AddEntity();
    Entity_ *hero = EntityManager::Get()->GetEntity(heroKey);
    strcpy(hero->name, "Hero");
    EntityManager::Get()->AddComponent<PlayerCMP>(heroKey);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(heroKey);
    transformCmp->Initialize(Vec3(0, 8, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f));

    PhysicsCMP *physicsCmp = EntityManager::Get()->AddComponent<PhysicsCMP>(heroKey);
    physicsCmp->Initialize(Vec3(0, 8, 0), Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(heroKey);
    graphicsCmp->Initialize("hero.twm", "animVert.hlsl", "mapFrag.hlsl");

    AnimationCMP *animationCmp = EntityManager::Get()->AddComponent<AnimationCMP>(heroKey);
    animationCmp->Initialize("hero.twa");

    InputCMP *inputCmp = EntityManager::Get()->AddComponent<InputCMP>(heroKey);

    Cylinder cylinder = {};
    cylinder.c = Vec3(0, 30, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = EntityManager::Get()->AddComponent<CollisionCMP>(heroKey);
    collisionCmp->Initialize(cylinder);

    FireSpellCMP *fireCmp = EntityManager::Get()->AddComponent<FireSpellCMP>(heroKey);
    fireCmp->Initialize();

    return heroKey; 
}

static SlotmapKey CreateOrc(char *name, Vec3 pos, BehaviorTree *bhTree = nullptr) {
    Input *input = PlatformManager::Get()->GetInput();

    SlotmapKey orc = EntityManager::Get()->AddEntity();
    Entity_ *orcPtr = EntityManager::Get()->GetEntity(orc);
    strcpy(orcPtr->name, name);

    EntityManager::Get()->AddComponent<EnemyCMP>(orc);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(orc);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    PhysicsCMP *physicsCmp = EntityManager::Get()->AddComponent<PhysicsCMP>(orc);
    physicsCmp->Initialize(pos, Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(orc);
    graphicsCmp->Initialize("orc.twm", "animVert.hlsl", "mapFrag.hlsl");

    AnimationCMP *animationCmp = EntityManager::Get()->AddComponent<AnimationCMP>(orc);
    animationCmp->Initialize("hero.twa");

    Cylinder cylinder = {};
    cylinder.c = pos;
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = EntityManager::Get()->AddComponent<CollisionCMP>(orc);
    collisionCmp->Initialize(cylinder);

    AiCMP *aiCmp = EntityManager::Get()->AddComponent<AiCMP>(orc);
    aiCmp->Initialize(STEERING_BEHAVIOR_FACE, 0.75f, 2.0f, true, bhTree);

    return orc;
    
}

static SlotmapKey CreateRain(char *name, Vec3 pos) {

    SlotmapKey rain = EntityManager::Get()->AddEntity();

    Entity_ *rainPtr = EntityManager::Get()->GetEntity(rain);
    strcpy(rainPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(rain);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    ParticleCMP *parCmp = EntityManager::Get()->AddComponent<ParticleCMP>(rain);
    parCmp->Initialize(1000, 
            "soRainVert.hlsl", "soRainFrag.hlsl", "soRainGeo.hlsl",
            "dwRainVert.hlsl", "dwRainFrag.hlsl", "dwRainGeo.hlsl",
            "raindrop1.png");

    return rain;
}

static SlotmapKey CreateLava(char *name, Vec3 pos) {

    SlotmapKey rain = EntityManager::Get()->AddEntity();

    Entity_ *rainPtr = EntityManager::Get()->GetEntity(rain);
    strcpy(rainPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(rain);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    ParticleCMP *parCmp = EntityManager::Get()->AddComponent<ParticleCMP>(rain);
    parCmp->Initialize(2000, 
            "soLavaVert.hlsl", "soLavaFrag.hlsl", "soLavaGeo.hlsl",
            "dwLavaVert.hlsl", "dwLavaFrag.hlsl", "dwLavaGeo.hlsl",
            "flare.png");

    return rain;
}


static SlotmapKey CreateMovingPlatform(char *name, Vec3 scale, Vec3 a, Vec3 b) {
    SlotmapKey platform = EntityManager::Get()->AddEntity();
    Entity_ *platformPtr = EntityManager::Get()->GetEntity(platform);
    strcpy(platformPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(platform);
    transformCmp->Initialize(a, Vec3(), scale);

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(platform);
    graphicsCmp->Initialize("cube.twm", "staticVert.hlsl", "staticFrag.hlsl");

    // Collider
    ConvexHull convexHull {};
    convexHull.points = CreateCube();
    convexHull.count  = ARRAY_LENGTH(gCube);
    MapImporter::Entity entity {};
    entity.faces = (MapImporter::EntityFace *)MemoryManager::Get()->AllocStaticMemory(sizeof(MapImporter::EntityFace) * 6, 1);
    memcpy(entity.faces, gCubeFaces, sizeof(MapImporter::EntityFace) * 6);
    entity.facesCount = 6;

    CollisionCMP *collisionCmp = EntityManager::Get()->AddComponent<CollisionCMP>(platform);
    collisionCmp->Initialize(convexHull, entity);

    MovingPlatformCMP *movingCmp = EntityManager::Get()->AddComponent<MovingPlatformCMP>(platform);
    movingCmp->Initialize(a, b);

    return platform;
}

static SlotmapKey CreateBox(char *name, Vec3 pos, Vec3 scale) {
    SlotmapKey box = EntityManager::Get()->AddEntity();
    Entity_ *boxPtr = EntityManager::Get()->GetEntity(box);
    strcpy(boxPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(box);
    transformCmp->Initialize(pos, Vec3(), scale);

    PhysicsCMP *physicsCmp = EntityManager::Get()->AddComponent<PhysicsCMP>(box);
    physicsCmp->Initialize(pos, Vec3(), Vec3());
    physicsCmp->physics.orientation = (f32)(PI*0.5);

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(box);
    graphicsCmp->Initialize("cube.twm", "staticVert.hlsl", "staticFrag.hlsl");

    // Collider
    ConvexHull convexHull {};
    convexHull.points = CreateCube();
    convexHull.count  = ARRAY_LENGTH(gCube);
    MapImporter::Entity entity {};
    entity.faces = (MapImporter::EntityFace *)MemoryManager::Get()->AllocStaticMemory(sizeof(MapImporter::EntityFace) * 6, 1);
    memcpy(entity.faces, gCubeFaces, sizeof(MapImporter::EntityFace) * 6);
    entity.facesCount = 6;

    CollisionCMP *collisionCmp = EntityManager::Get()->AddComponent<CollisionCMP>(box);
    collisionCmp->Initialize(convexHull, entity);

    EntityManager::Get()->AddComponent<MovableBoxCMP>(box);

    return box;
}

static SlotmapKey CreateGem(SlotmapKey whoTriggerThis, Vec3 position) {

    SlotmapKey gem = EntityManager::Get()->AddEntity();
    Entity_ *gemPtr = EntityManager::Get()->GetEntity(gem);
    strcpy(gemPtr->name, "gem");
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(gem);
    transformCmp->Initialize(position, Vec3(), Vec3(0.2f, 0.2f, 0.2f));

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(gem);
    graphicsCmp->Initialize("gem.twm", "staticVert.hlsl", "staticFrag.hlsl");

    Cylinder cylinder = {};
    cylinder.c = position;
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.2f;
    cylinder.n = 0.25f;
    TriggerCMP *triggerCmp = EntityManager::Get()->AddComponent<TriggerCMP>(gem);
    triggerCmp->Initialize(whoTriggerThis, cylinder);

    GemCMP *gemCmp = EntityManager::Get()->AddComponent<GemCMP>(gem);

    i32 value = rand() % (50 + 1) + 50;

    gemCmp->Initialize(value);

    return gem;
}





bool Level::DeleteEntity(SlotmapKey entityKey) {
    entitiesToRemove.Push(entityKey);
    return true;
}

void Level::DestroyEntityAndComponents(SlotmapKey entityKey) {
    i32 indexToDelete = -1;
    for(i32 i = 0; i < entities.size; ++i) {
        if(entityKey.gen == entities[i].gen) {
            indexToDelete = i;
            break;
        }
    }

    if(indexToDelete == -1) {
        return;
    } else {
        EntityManager::Get()->DeleteEntity(entityKey); 
        entities[indexToDelete] = entities[entities.size - 1];
        --entities.size;
        return;
    }
}


void Level::DeleteEntitiesToRemove() {
    for(i32 i = 0; i < entitiesToRemove.size; ++i) {
        DestroyEntityAndComponents(entitiesToRemove[i]);
    }
}


void Level::Initialize(char *levelPath, Camera *camera) {

    srand(time(NULL));
    memory.BeginFrame();
    
    this->camera = camera;
    entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
    EntityManager::Get()->Initialize();
    AddTypesToEntityManager();

    // Load the BehaviorTree
    bhTree.Initialize();
    bhTree.AddNode<BehaviorSequence>(
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0, -6)),
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0, -6))
    );
    
    printf("deserializing level!! ...\n");

    Tokenizer t;
    t.Begin(levelPath);
    Deserialize(&t);
    t.End();

    heroKey = entities[0];
    TransformCMP *heroTransform = EntityManager::Get()->GetComponent<TransformCMP>(heroKey);
    gBlackBoard.target = &heroTransform->pos;


    //entities.Push(CreateBox("box1", Vec3(0, 3, 0), Vec3(1, 1, 1)));
    /*
    entities.Push(CreateBox("box1", Vec3(0, 3, 0), Vec3(1, 1, 1)));
    entities.Push(CreateBox("box2", Vec3(0, 3, 0), Vec3(1, 1, 1)));
    entities.Push(CreateBox("box3", Vec3(0, 3, 0), Vec3(1, 1, 1)));
    entities.Push(CreateBox("box4", Vec3(0, 3, 0), Vec3(1, 1, 1)));
    */
    
}

#if 0
void Level::Initialize(char *mapFilePath, Camera *camera) {

    srand(time(NULL));
    memory.BeginFrame();
    
    this->camera = camera;
    strcpy(mapName, mapFilePath);

    entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
    
    EntityManager::Get()->Initialize();
    AddTypesToEntityManager();

    // NOTE Load Map ------------------------------------------------------------------------------------------
    map.Initialize(mapFilePath);

    // Load the BehaviorTree
    bhTree.Initialize();
    bhTree.AddNode<BehaviorSequence>(
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0, -6)),
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0, -6))
    );

    // NOTE Load entities ------------------------------------------------------------------------------------    
    entities.Push(CreateHero(camera));

    entities.Push(CreateOrc("orc_1", Vec3(0, 4, 20)));
    entities.Push(CreateOrc("orc_2", Vec3(0, 4, 15)));
    entities.Push(CreateOrc("orc_3", Vec3(0, 4, 8), &bhTree));

    entities.Push(CreateRain("rain", Vec3())); 
    entities.Push(CreateLava("lava", Vec3(0, -11.11f, 73.5f))); 

    entities.Push(CreateMovingPlatform("mov_plat_1", Vec3(2, 0.5f, 2), Vec3(  1, -0.5f, 67),    Vec3(22,  -0.5f, 67)));
    entities.Push(CreateMovingPlatform("mov_plat_2", Vec3(4, 0.5f, 2), Vec3(  0, -0.5f, 78),    Vec3(26,  -0.5f, 78)));
    entities.Push(CreateMovingPlatform("mov_plat_3", Vec3(4, 0.5f, 2), Vec3( 22, -0.5f, 86),    Vec3(0, -0.5f, 86)));
    entities.Push(CreateMovingPlatform("mov_plat_4", Vec3(4, 0.5f, 2), Vec3(-22,  0.0f, 88.2f), Vec3(0,  0.0f, 88.2f)));

    //entities.Push(CreateGem(em, entities[0], *gemModel, Vec3(8, 2.2f, 8), statShader));
    for(i32 y = 0; y < 10; y++) {
        for(i32 x = 0; x < 10; x++) {
            entities.Push(CreateGem(entities[0], Vec3(-4 + (f32)x, 2.2f, 8 + (f32)y)));
        }
    }

    heroKey = entities[0];
    TransformCMP *heroTransform = EntityManager::Get()->GetComponent<TransformCMP>(heroKey);
    gBlackBoard.target = &heroTransform->pos;
}
#endif

#include "tokenizer.h"

void Level::Terminate() {
    
    map.Terminate();
    
    AnimationManager::Get()->ClearAssets();
    ModelManager::Get()->ClearAssets();
    TextureManager::Get()->ClearAssets();

    EntityManager::Get()->Terminate();

    memory.EndFrame();
}


void Level::BeginFrame(f32 dt) {
    entitiesToRemove.Begin(ENTITY_ARRAY_MAX_SIZE);
}

void Level::EndFrame(f32 dt) {
    DeleteEntitiesToRemove();
    entitiesToRemove.End();
}

void Level::Update(f32 dt) {

    if(dt > 0) gameTime += dt;

    // TODO: Remove this pointer to EntityManager
    EntityManager em = *EntityManager::Get();
    
    physicsSys.PreUpdate(em, dt);
    inputSys.Update(em, camera, dt);
    aiSys.Update(em, dt);


    levitationSpellSys.Update(em, camera, gameTime, dt);

    physicsSys.Update(em, dt);
    movingPlatformSys.Update(em, dt);
    triggerSys.Update(em, dt);
    collisionSys.Update(em, &map, dt);
    physicsSys.PostUpdate(em, dt);
    animationSys.Update(em, dt);
    gemSys.Update(em , this, dt);
    transformSys.Update(em);

    TransformCMP *heroTransform = EntityManager::Get()->GetComponent<TransformCMP>(heroKey);
    camera->SetTarget(heroTransform->pos);

    particleSys.Update(em, camera->pos, gameTime, dt);
    fireSpellSys.Update(em, this, camera->pos, gameTime, dt);
}

void Level::Render() {
    map.Render();

    // TODO: Remove this pointer to EntityManager
    EntityManager em = *EntityManager::Get();
    graphicsSys.Update(em);
    particleSys.Render(em);
    fireSpellSys.Render(em);
    levitationSpellSys.Render(em);
}

void Level::Serialize(Serializer *s) {
    WriteBeginObject(s, "level");
    Write(s, "map", mapName);
    Write(s, "num_entities", (i32)entities.size);
    WriteBeginArray(s, "entities");
    for(i32 i = 0; i <  entities.size; ++i) {
        Entity_ *entity = EntityManager::Get()->GetEntity(entities[i]);
        entity->Serialize(s);
    }
    WriteEndArray(s);

    WriteEndObject(s);
}

void Level::Deserialize(Tokenizer *t) {
    ReadBeginObject(t, "level");
    Read(t, "map", mapName, MAX_LEVEL_MAP_NAME_SIZE);
    map.Initialize(mapName);
    i32 numEntities = 0;
    Read(t, "num_entities", &numEntities);
    ReadBeginArray(t, "entities");
    for(i32 i = 0; i <  numEntities; ++i) {
        SlotmapKey key = EntityManager::Get()->AddEntity();
        entities.Push(key);
        Entity_ *entity = EntityManager::Get()->GetEntity(key);
        entity->Deserialize(t);
    }
    ReadEndArray(t);

    ReadEndObject(t);
}
