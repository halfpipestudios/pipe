#include "level.h"
#include "geometry.h"
#include "steering_behaviors.h"

#include <float.h>
#include <stdio.h>

#include "cmp/transform_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/ai_cmp.h"

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


void Map::Render(Shader shader) {

    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(scale, scale, scale));
    GraphicsManager::Get()->BindTextureBuffer(texture);
    GraphicsManager::Get()->DrawVertexBuffer(vertexBuffer, shader);

}


static SlotmapKey CreateHero(EntityManager& em, Model& model, Shader shader,
                           AnimationClipSet *animationClipSet, Camera *camera) {

    Input *input = PlatformManager::Get()->GetInput();

    SlotmapKey heroKey = em.AddEntity();
    Entity_ *hero = em.GetEntity(heroKey);
    hero->name = "Hero";
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(heroKey);
    transformCmp->Initialize(Vec3(0, 30, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f));

    PhysicsCMP *physicsCmp = em.AddComponent<PhysicsCMP>(heroKey);
    physicsCmp->Initialize(Vec3(0, 30, 0), Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(heroKey);
    graphicsCmp->Initialize(model, shader);

    // AnimationCMP *animationCmp = em.AddComponent<AnimationCMP>(heroKey);
    // animationCmp->Initialize(animationClipSet);

    InputCMP *inputCmp = em.AddComponent<InputCMP>(heroKey);
    inputCmp->Initialize(input, camera);

    Cylinder cylinder = {};
    cylinder.c = Vec3(0, 30, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = em.AddComponent<CollisionCMP>(heroKey);
    collisionCmp->Initialize(cylinder);

    return heroKey;
    
}

static SlotmapKey CreateOrc(EntityManager& em,
                          char *name,
                          Vec3 pos,
                          Model& model, Shader shader,
                          AnimationClipSet *animationClipSet,
                          BehaviorTree *bhTree = nullptr) {

    Input *input = PlatformManager::Get()->GetInput();

    SlotmapKey orc = em.AddEntity();
    Entity_ *orcPtr = em.GetEntity(orc);
    orcPtr->name = name;
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(orc);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    PhysicsCMP *physicsCmp = em.AddComponent<PhysicsCMP>(orc);
    physicsCmp->Initialize(pos, Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(orc);
    graphicsCmp->Initialize(model, shader);

    // AnimationCMP *animationCmp = em.AddComponent<AnimationCMP>(orc);
    // animationCmp->Initialize(animationClipSet);

    Cylinder cylinder = {};
    cylinder.c = pos;
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = em.AddComponent<CollisionCMP>(orc);
    collisionCmp->Initialize(cylinder);

    AiCMP *aiCmp = em.AddComponent<AiCMP>(orc);
    aiCmp->Initialize(STEERING_BEHAVIOR_ARRIVE, 0.75f, 2.0f, true, bhTree);

    return orc;
    
}


static SlotmapKey CreateMovingPlatform(EntityManager& em, char *name, Vec3 scale, Vec3 a, Vec3 b, Shader shader) {
    SlotmapKey platform = em.AddEntity();
    Entity_ *platformPtr = em.GetEntity(platform);
    platformPtr->name = name;
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(platform);
    transformCmp->Initialize(a, Vec3(), scale);

    Model *model = ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset("cube.twm"));
    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(platform);
    graphicsCmp->Initialize(*model, shader);

    // Collider
    ConvexHull convexHull {};
    convexHull.points = CreateCube();
    convexHull.count  = ARRAY_LENGTH(gCube);
    MapImporter::Entity entity {};
    entity.faces = (MapImporter::EntityFace *)MemoryManager::Get()->AllocStaticMemory(sizeof(MapImporter::EntityFace) * 6, 1);
    memcpy(entity.faces, gCubeFaces, sizeof(MapImporter::EntityFace) * 6);
    entity.facesCount = 6;

    CollisionCMP *collisionCmp = em.AddComponent<CollisionCMP>(platform);
    collisionCmp->Initialize(convexHull, entity);

    MovingPlatformCMP *movingCmp = em.AddComponent<MovingPlatformCMP>(platform);
    movingCmp->Initialize(a, b);

    return platform;
}

bool Level::DeleteEntity(SlotmapKey entityKey) {
    i32 indexToDelete = -1;
    for(i32 i = 0; i < entities.size; ++i) {
        if(entityKey.gen == entities[i].gen) {
            indexToDelete = i;
            break;
        }
    }

    if(indexToDelete == -1) {
        return false;
    } else {
        em.DeleteEntity(entityKey); 
        entities[indexToDelete] = entities[entities.size - 1];
        --entities.size;
        return true;
    }
}

void Level::Initialize(char *mapFilePath, Shader statShader, Shader animShader) {

    memory.BeginFrame();

    entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
    
    em.Initialize();
    em.AddComponentType<TransformCMP>();
    em.AddComponentType<GraphicsCMP>();
    em.AddComponentType<PhysicsCMP>();
    em.AddComponentType<AnimationCMP>();
    em.AddComponentType<InputCMP>();
    em.AddComponentType<CollisionCMP>();
    em.AddComponentType<MovingPlatformCMP>();
    em.AddComponentType<AiCMP>();

    // NOTE Load Map ------------------------------------------------------------------------------------------
    map.Initialize(mapFilePath);

    // Load the BehaviorTree
    bhTree.Initialize();
    bhTree.AddNode<BehaviorSequence>(
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3( -8, 0,  8)),
        bhTree.AddNode<BehaviorArrive>(Vec3(-16, 0, -6)),
        bhTree.AddNode<BehaviorArrive>(Vec3(  8, 0, -6))
    );    

    // NOTE Load entities ------------------------------------------------------------------------------------    
    
    AnimationClipSet *heroAnim = AnimationManager::Get()->Dereference(AnimationManager::Get()->GetAsset("hero.twa"));

    Model *heroModel = ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset("hero.twm"));
    Model *orcModel = ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset("orc.twm"));

    camera.Initialize();

    entities.Push(CreateHero(em, *heroModel, animShader, heroAnim, &camera));
    entities.Push(CreateOrc(em, "orc_1",  Vec3(10, 4, 10), *orcModel, animShader, heroAnim));
    entities.Push(CreateOrc(em, "orc_2",  Vec3(10, 4, 15), *orcModel, animShader, heroAnim));
    entities.Push(CreateOrc(em, "orc_3", Vec3(10, 4, 8),  *orcModel, animShader, heroAnim, &bhTree));
    entities.Push(CreateMovingPlatform(em, "mov_plat_1", Vec3(2, 0.5f, 2), Vec3(10,  3, -5), Vec3(10,  3, 5), statShader));
    entities.Push(CreateMovingPlatform(em, "mov_plat_2", Vec3(2, 0.5f, 4), Vec3(14, 10,  0), Vec3(14,  3, 0), statShader));
    entities.Push(CreateMovingPlatform(em, "mov_plat_3", Vec3(2, 0.5f, 2), Vec3(14, 10,  4), Vec3(14, 20, 4), statShader));

    heroKey = entities[0];

    TransformCMP *heroTransform = em.GetComponent<TransformCMP>(heroKey);
    gBlackBoard.target = &heroTransform->pos;
}

void Level::Terminate() {
    
    map.Terminate();
    
    AnimationManager::Get()->ClearAssets();
    ModelManager::Get()->ClearAssets();
    TextureManager::Get()->ClearAssets();

    memory.EndFrame();
}

void Level::Update(f32 dt) {

    Input *input = PlatformManager::Get()->GetInput();

    physicsSys.PreUpdate(em, dt);
    inputSys.Update(em, dt);
    aiSys.Update(em, dt);
    physicsSys.Update(em, dt);
    movingPlatformSys.Update(em, dt);
    collisionSys.Update(em, &map, dt);
    physicsSys.PostUpdate(em, dt);
    animationSys.Update(em, dt);
    transformSys.Update(em);

    TransformCMP *heroTransform = em.GetComponent<TransformCMP>(heroKey);
    camera.target = heroTransform->pos;
    camera.ProcessMovement(input, &map, dt);
    camera.SetViewMatrix();
}

void Level::Render(Shader mapShader) {
    map.Render(mapShader);
    graphicsSys.Update(em);
}

