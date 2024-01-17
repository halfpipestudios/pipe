#include "level.h"
#include "geometry.h"
#include "steering_behaviors.h"

#include <float.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "cmp/transform_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/ai_cmp.h"
#include "cmp/trigger_cmp.h"
#include "cmp/gem_cmp.h"
#include "cmp/particle_cmp.h"
#include "cmp/fire_spell_cmp.h"
#include "cmp/player_cmp.h"
#include "cmp/enemy_cmp.h"

#include "mgr/texture_manager.h"
#include "mgr/model_manager.h"
#include "mgr/animation_manager.h"

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

void Map::Initialize(char *filename, Shader mapShader) {

    MapImporter mapImporter;
    mapImporter.LoadMapFromFile(filename);
    MapImporter::VertexArray mapVertices = mapImporter.GetVertices();
    MapImporter::TextureArray mapTextures = mapImporter.GetTextures();
    
    covexHulls = mapImporter.GetConvexHulls();
    entities = mapImporter.GetEntities();
    vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count, sizeof(VertexMap));
    texture = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    scale = 1.0f/32.0f;
    shader = mapShader;
}

void Map::Terminate() {
    GraphicsManager::Get()->DestroyTextureBuffer(texture);
    GraphicsManager::Get()->DestroyVertexBuffer(vertexBuffer);
}


void Map::Render() {
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));
    GraphicsManager::Get()->BindTextureBuffer(texture);
    GraphicsManager::Get()->DrawVertexBuffer(vertexBuffer, shader);
}


static SlotmapKey CreateHero(Shader shader, AnimationClipSet *animationClipSet, Camera *camera,
                             Shader soShader, GeometryShader soGeoShader,
                             Shader dwShader, GeometryShader dwGeoShader) {

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
    graphicsCmp->Initialize("hero.twm", shader);

    AnimationCMP *animationCmp = EntityManager::Get()->AddComponent<AnimationCMP>(heroKey);
    animationCmp->Initialize(animationClipSet);

    InputCMP *inputCmp = EntityManager::Get()->AddComponent<InputCMP>(heroKey);
    inputCmp->Initialize(input, camera);

    Cylinder cylinder = {};
    cylinder.c = Vec3(0, 30, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = EntityManager::Get()->AddComponent<CollisionCMP>(heroKey);
    collisionCmp->Initialize(cylinder);

    FireSpellCMP *fireCmp = EntityManager::Get()->AddComponent<FireSpellCMP>(heroKey);
    fireCmp->Initialize(soShader, soGeoShader,
                        dwShader, dwGeoShader,
                        TextureManager::Get()->GetAsset("fire_test.png"));

    return heroKey;
    
}

static SlotmapKey CreateOrc(char *name,
                            Vec3 pos,
                            Shader shader,
                            AnimationClipSet *animationClipSet,
                            
                            u32 maxParticles,
                            Shader soShader, GeometryShader soGeoShader,
                            Shader dwShader, GeometryShader dwGeoShader,
                            Handle texture,

                            BehaviorTree *bhTree = nullptr) {

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
    graphicsCmp->Initialize("orc.twm", shader);

    AnimationCMP *animationCmp = EntityManager::Get()->AddComponent<AnimationCMP>(orc);
    animationCmp->Initialize(animationClipSet);

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

static SlotmapKey CreateRain(char *name, Vec3 pos,
                             Shader soRainShader, GeometryShader soRainGeoShader,
                             Shader dwRainShader, GeometryShader dwRainGeoShader) {

    SlotmapKey rain = EntityManager::Get()->AddEntity();

    Entity_ *rainPtr = EntityManager::Get()->GetEntity(rain);
    strcpy(rainPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(rain);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    ParticleCMP *parCmp = EntityManager::Get()->AddComponent<ParticleCMP>(rain);
    parCmp->Initialize(1000, 
            soRainShader, soRainGeoShader,
            dwRainShader, dwRainGeoShader,
            TextureManager::Get()->GetAsset("raindrop1.png"));

    return rain;
}

static SlotmapKey CreateLava(char *name, Vec3 pos,
                             Shader soRainShader, GeometryShader soRainGeoShader,
                             Shader dwRainShader, GeometryShader dwRainGeoShader) {

    SlotmapKey rain = EntityManager::Get()->AddEntity();

    Entity_ *rainPtr = EntityManager::Get()->GetEntity(rain);
    strcpy(rainPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(rain);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    ParticleCMP *parCmp = EntityManager::Get()->AddComponent<ParticleCMP>(rain);
    parCmp->Initialize(2000, 
            soRainShader, soRainGeoShader,
            dwRainShader, dwRainGeoShader,
            TextureManager::Get()->GetAsset("flare.png"));

    return rain;
}


static SlotmapKey CreateMovingPlatform(char *name, Vec3 scale, Vec3 a, Vec3 b, Shader shader) {
    SlotmapKey platform = EntityManager::Get()->AddEntity();
    Entity_ *platformPtr = EntityManager::Get()->GetEntity(platform);
    strcpy(platformPtr->name, name);
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(platform);
    transformCmp->Initialize(a, Vec3(), scale);

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(platform);
    graphicsCmp->Initialize("cube.twm", shader);

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

static SlotmapKey CreateGem(SlotmapKey whoTriggerThis, Vec3 position, Shader shader) {

    SlotmapKey gem = EntityManager::Get()->AddEntity();
    Entity_ *gemPtr = EntityManager::Get()->GetEntity(gem);
    strcpy(gemPtr->name, "gem");
    
    TransformCMP *transformCmp = EntityManager::Get()->AddComponent<TransformCMP>(gem);
    transformCmp->Initialize(position, Vec3(), Vec3(0.2f, 0.2f, 0.2f));

    GraphicsCMP *graphicsCmp = EntityManager::Get()->AddComponent<GraphicsCMP>(gem);
    graphicsCmp->Initialize("gem.twm", shader);

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

void Level::Initialize(char *mapFilePath, Camera *camera,
            Shader mapShader, Shader statShader, Shader animShader) {

    srand(time(NULL));
    
    memory.BeginFrame();
    
    this->camera = camera;

    entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
    
    EntityManager::Get()->Initialize();
    EntityManager::Get()->AddComponentType<TransformCMP>();
    EntityManager::Get()->AddComponentType<GraphicsCMP>();
    EntityManager::Get()->AddComponentType<PhysicsCMP>();
    EntityManager::Get()->AddComponentType<AnimationCMP>();
    EntityManager::Get()->AddComponentType<InputCMP>();
    EntityManager::Get()->AddComponentType<CollisionCMP>();
    EntityManager::Get()->AddComponentType<MovingPlatformCMP>();
    EntityManager::Get()->AddComponentType<AiCMP>();
    EntityManager::Get()->AddComponentType<TriggerCMP>();
    EntityManager::Get()->AddComponentType<GemCMP>();
    EntityManager::Get()->AddComponentType<ParticleCMP>();
    EntityManager::Get()->AddComponentType<FireSpellCMP>();
    EntityManager::Get()->AddComponentType<PlayerCMP>();
    EntityManager::Get()->AddComponentType<EnemyCMP>();

    // NOTE Load Map ------------------------------------------------------------------------------------------
    map.Initialize(mapFilePath, mapShader);

    // Load the BehaviorTree
    bhTree.Initialize();
    bhTree.AddNode<BehaviorSequence>(
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0, -6)),
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0, -6))
    );    

    // NOTE Load entities ------------------------------------------------------------------------------------    
    
    AnimationClipSet *heroAnim = AnimationManager::Get()->Dereference(AnimationManager::Get()->GetAsset("hero.twa"));

    // Shaders for FIRE particle system
    soFireShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soFireVert.hlsl", "./data/shaders/soFireFrag.hlsl");
    soFireGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soFireGeo.hlsl");
    dwFireShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwFireVert.hlsl", "./data/shaders/dwFireFrag.hlsl");
    dwFireGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwFireGeo.hlsl");

    // Shaders for RAIN particle system
    soRainShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soRainVert.hlsl", "./data/shaders/soRainFrag.hlsl");
    soRainGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soRainGeo.hlsl");
    dwRainShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwRainVert.hlsl", "./data/shaders/dwRainFrag.hlsl");
    dwRainGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwRainGeo.hlsl");

    // Shaders for LAVA particle system
    soLavaShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soLavaVert.hlsl", "./data/shaders/soLavaFrag.hlsl");
    soLavaGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soLavaGeo.hlsl");
    dwLavaShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwLavaVert.hlsl", "./data/shaders/dwLavaFrag.hlsl");
    dwLavaGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwLavaGeo.hlsl");

    // Shaders for SPELL particle system
    soSpellShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soSpellVert.hlsl", "./data/shaders/soSpellFrag.hlsl");
    soSpellGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soSpellGeo.hlsl");
    dwSpellShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwSpellVert.hlsl", "./data/shaders/dwSpellFrag.hlsl");
    dwSpellGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwSpellGeo.hlsl");

    // Shaders for SHOOT SPELL particle system
    soShootShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/soShootVert.hlsl", "./data/shaders/soShootFrag.hlsl");
    soShootGeoShader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput("./data/shaders/soShootGeo.hlsl");
    dwShootShader = GraphicsManager::Get()->CreateShaderParticle("./data/shaders/dwShootVert.hlsl", "./data/shaders/dwShootFrag.hlsl");
    dwShootGeoShader = GraphicsManager::Get()->CreateGeometryShader("./data/shaders/dwShootGeo.hlsl");

    entities.Push(CreateHero(animShader, heroAnim, camera, soShootShader, soShootGeoShader, dwShootShader, dwShootGeoShader));

    entities.Push(CreateOrc("orc_1",  Vec3(0, 4, 20), animShader, heroAnim, 
                1000, soFireShader, soFireGeoShader, dwFireShader, dwFireGeoShader, TextureManager::Get()->GetAsset("flare.png")));
    entities.Push(CreateOrc("orc_2",  Vec3(0, 4, 15), animShader, heroAnim, 
                1000, soFireShader, soFireGeoShader, dwFireShader, dwFireGeoShader, TextureManager::Get()->GetAsset("raindrop1.png")));
    entities.Push(CreateOrc("orc_3", Vec3(0, 4, 8),  animShader, heroAnim,
                1000, soFireShader, soFireGeoShader, dwFireShader, dwFireGeoShader, TextureManager::Get()->GetAsset("flare.png"), &bhTree));

    entities.Push(CreateRain("rain", Vec3(), soRainShader, soRainGeoShader, dwRainShader, dwRainGeoShader)); 
    entities.Push(CreateLava("lava", Vec3(0, -11.11f, 73.5f), soLavaShader, soLavaGeoShader, dwLavaShader, dwLavaGeoShader)); 

    entities.Push(CreateMovingPlatform("mov_plat_1", Vec3(2, 0.5f, 2), Vec3(  1, -0.5f, 67),    Vec3(22,  -0.5f, 67), statShader));
    entities.Push(CreateMovingPlatform("mov_plat_2", Vec3(4, 0.5f, 2), Vec3(  0, -0.5f, 78),    Vec3(26,  -0.5f, 78), statShader));
    entities.Push(CreateMovingPlatform("mov_plat_3", Vec3(4, 0.5f, 2), Vec3( 22, -0.5f, 86),    Vec3(0, -0.5f, 86),  statShader));
    entities.Push(CreateMovingPlatform("mov_plat_4", Vec3(4, 0.5f, 2), Vec3(-22,  0.0f, 88.2f), Vec3(0,  0.0f, 88.2f),  statShader));

    //entities.Push(CreateGem(em, entities[0], *gemModel, Vec3(8, 2.2f, 8), statShader));
    for(i32 y = 0; y < 10; y++) {
        for(i32 x = 0; x < 10; x++) {
            entities.Push(CreateGem(entities[0], Vec3(-4 + (f32)x, 2.2f, 8 + (f32)y), statShader));
        }
    }

    heroKey = entities[0];

    TransformCMP *heroTransform = EntityManager::Get()->GetComponent<TransformCMP>(heroKey);
    gBlackBoard.target = &heroTransform->pos;
}

#include "tokenizer.h"

void Level::Terminate() {
    
    map.Terminate();
    
    AnimationManager::Get()->ClearAssets();
    ModelManager::Get()->ClearAssets();
    TextureManager::Get()->ClearAssets();

    GraphicsManager::Get()->DestroyShader(soFireShader);
    GraphicsManager::Get()->DestroyGeometryShader(soFireGeoShader);
    GraphicsManager::Get()->DestroyShader(dwFireShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwFireGeoShader);

    GraphicsManager::Get()->DestroyShader(soRainShader);
    GraphicsManager::Get()->DestroyGeometryShader(soRainGeoShader);
    GraphicsManager::Get()->DestroyShader(dwRainShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwRainGeoShader);

    GraphicsManager::Get()->DestroyShader(soLavaShader);
    GraphicsManager::Get()->DestroyGeometryShader(soLavaGeoShader);
    GraphicsManager::Get()->DestroyShader(dwLavaShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwLavaGeoShader);

    GraphicsManager::Get()->DestroyShader(soSpellShader);
    GraphicsManager::Get()->DestroyGeometryShader(soSpellGeoShader);
    GraphicsManager::Get()->DestroyShader(dwSpellShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwSpellGeoShader);

    GraphicsManager::Get()->DestroyShader(soShootShader);
    GraphicsManager::Get()->DestroyGeometryShader(soShootGeoShader);
    GraphicsManager::Get()->DestroyShader(dwShootShader);
    GraphicsManager::Get()->DestroyGeometryShader(dwShootGeoShader);

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
    inputSys.Update(em, dt);
    aiSys.Update(em, dt);
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
}

void Level::Serialize(Serializer *s) {
    WriteBeginObject(s, "level");
    
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
    
    i32 numEntities = 0;
    Read(t, "num_entities", &numEntities);
    ReadBeginArray(t, "entities");
    for(i32 i = 0; i <  numEntities; ++i) {
        SlotmapKey key = EntityManager::Get()->AddEntity();
        Entity_ *entity = EntityManager::Get()->GetEntity(key);
        entity->Deserialize(t);
    }
    ReadEndArray(t);

    ReadEndObject(t);
}
