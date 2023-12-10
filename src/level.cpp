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

static Entity_ *CreateHero(EntityManager& em, Model& model, Shader shader,
                           AnimationClipSet *animationClipSet, Camera *camera) {

    Input *input = PlatformManager::Get()->GetInput();

    Entity_ *hero = em.AddEntity();
    hero->name = "Hero";
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(hero);
    transformCmp->Initialize(Vec3(0, 30, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f));

    PhysicsCMP *physicsCmp = em.AddComponent<PhysicsCMP>(hero);
    physicsCmp->Initialize(Vec3(0, 30, 0), Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(hero);
    graphicsCmp->Initialize(model, shader);

    AnimationCMP *animationCmp = em.AddComponent<AnimationCMP>(hero);
    animationCmp->Initialize(animationClipSet);

    InputCMP *inputCmp = em.AddComponent<InputCMP>(hero);
    inputCmp->Initialize(input, camera);

    Cylinder cylinder = {};
    cylinder.c = Vec3(0, 30, 0);
    cylinder.u = Vec3(0, 1, 0);
    cylinder.radii = 0.3f;
    cylinder.n = 0.75f;
    CollisionCMP *collisionCmp = em.AddComponent<CollisionCMP>(hero);
    collisionCmp->Initialize(cylinder);

    return hero;
    
}

static Entity_ *CreateOrc(EntityManager& em,
                          char *name,
                          Vec3 pos,
                          Model& model, Shader shader,
                          AnimationClipSet *animationClipSet,
                          BehaviorTree *bhTree = nullptr) {

    Input *input = PlatformManager::Get()->GetInput();

    Entity_ *orc = em.AddEntity();
    orc->name = name;
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(orc);
    transformCmp->Initialize(pos, Vec3(), Vec3(1.0f, 1.0f, 1.0f));

    PhysicsCMP *physicsCmp = em.AddComponent<PhysicsCMP>(orc);
    physicsCmp->Initialize(pos, Vec3(), Vec3());

    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(orc);
    graphicsCmp->Initialize(model, shader);

    AnimationCMP *animationCmp = em.AddComponent<AnimationCMP>(orc);
    animationCmp->Initialize(animationClipSet);

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


static Entity_ *CreateMovingPlatform(EntityManager& em, char *name, Vec3 scale, Vec3 a, Vec3 b, Shader shader) {
    Entity_ *platform = em.AddEntity();
    platform->name = name;
    
    TransformCMP *transformCmp = em.AddComponent<TransformCMP>(platform);
    transformCmp->Initialize(a, Vec3(), scale);

    Mesh *mesh = (Mesh *)MemoryManager::Get()->AllocStaticMemory(sizeof(Mesh), 1); 
    mesh->texture = TextureManager::Get()->GetAsset("cool.png");
    mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(gCubeVertices, ARRAY_LENGTH(gCubeVertices), sizeof(Vertex));
    mesh->indexBuffer = nullptr;

    // load the model and add the mesh to the model
    Model model = {};
    model.type = MODEL_TYPE_STATIC;
    model.numMeshes = 1;
    model.meshes = mesh;

    GraphicsCMP *graphicsCmp = em.AddComponent<GraphicsCMP>(platform);
    graphicsCmp->Initialize(model, shader);

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

void Level::Initialize(char *mapFilePath, Shader statShader, Shader animShader) {

    memory.BeginFrame();
    
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

    Model *heroModel = ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset("hero.twm"));
    Model *orcModel = ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset("orc.twm"));

    camera.Initialize();

    // NOTE Load entities ------------------------------------------------------------------------------------    
    hero = CreateHero(em, *heroModel, animShader, &animationsSets[1], &camera);
    orc = CreateOrc(em, "orc_1",  Vec3(10, 4, 10), *orcModel, animShader, &animationsSets[1]);
    orc1 = CreateOrc(em, "orc_2", Vec3(10, 4, 8),  *orcModel, animShader, &animationsSets[1], &bhTree);
    platformHor  = CreateMovingPlatform(em, "mov_plat_1", Vec3(2, 0.5f, 2), Vec3(10,  3, -5), Vec3(10,  3, 5), statShader);
    platformVer0 = CreateMovingPlatform(em, "mov_plat_2", Vec3(2, 0.5f, 4), Vec3(14, 10,  0), Vec3(14,  3, 0), statShader);
    platformVer1 = CreateMovingPlatform(em, "mov_plat_3", Vec3(2, 0.5f, 2), Vec3(14, 10,  4), Vec3(14, 20, 4), statShader);

    TransformCMP *heroTransform = hero->GetComponent<TransformCMP>();
    gBlackBoard.target = &heroTransform->pos;

}

void Level::Terminate() {
    
    GraphicsManager::Get()->DestroyTextureBuffer(map.texture);
    GraphicsManager::Get()->DestroyVertexBuffer(map.vertexBuffer);

    ModelManager::Get()->ClearAssets();
    TextureManager::Get()->ClearAssets();

    memory.EndFrame();
}

void Level::Update(f32 dt) {

    Input *input = PlatformManager::Get()->GetInput();

    inputSys.Update(em, dt);
    aiSys.Update(em, dt);
    physicsSys.Update(em, dt);
    movingPlatformSys.Update(em, dt);
    collisionSys.Update(em, &map, dt);
    physicsSys.PostUpdate(em, dt);
    animationSys.Update(em, dt);
    transformSys.Update(em);

    TransformCMP *heroTransform = hero->GetComponent<TransformCMP>();
    camera.target = heroTransform->pos;
    camera.ProcessMovement(input, &map, dt);
    camera.SetViewMatrix();
}

void Level::Render(Shader mapShader) {
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(map.scale, map.scale, map.scale));
    GraphicsManager::Get()->BindTextureBuffer(map.texture);
    GraphicsManager::Get()->DrawVertexBuffer(map.vertexBuffer, mapShader);

    graphicsSys.Update(em);
}

