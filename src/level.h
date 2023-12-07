#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "common.h"
#include "allocators.h"
#include "map_importer.h"
#include "model_importer.h"
#include "camera.h"
#include "entity.h"

#include "behavior_tree.h"

#include "entity_manager.h"
#include "sys/transform_sys.inl"
#include "sys/physics_sys.inl"
#include "sys/graphics_sys.inl"
#include "sys/animation_sys.inl"
#include "sys/input_sys.inl"
#include "sys/collision_sys.inl"
#include "sys/moving_platform_sys.inl"
#include "sys/ai_sys.inl"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    TextureBuffer texture;
    f32 scale;
};

struct Level {
    
    void Initialize(char *mapFilePath, Shader mapShader, Shader animShader);
    void Terminate();

    void Update(f32 dt);
    void Render(Shader mapShader);

private:

    EntityManager em;
    GraphicsSys<EntityManager> graphicsSys;
    TransformSys<EntityManager> transformSys;
    PhysicsSys<EntityManager> physicsSys;
    AnimationSys<EntityManager> animationSys;
    InputSys<EntityManager> inputSys;
    CollisionSys<EntityManager> collisionSys;
    MovingPlatformSys<EntityManager> movingPlatformSys;
    AiSys<EntityManager> aiSys;


    Entity_ *manolo;

    Camera camera;
    
    Map map;

    Entity *hero;
    Entity *orc;
    Entity *orc1;
    Entity *platformHor;
    Entity *platformVer0;
    Entity *platformVer1;
 
    Entity *entitiesEnd;
    Entity *entities;
    ObjectAllocator<Entity> entitiesAllocator;

    Entity *AddEntity(const char *name);
    Entity *AddEntity(const char *name, Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader);
    Entity *AddMovingPlatform(const char *name, Vec3 scale, Vec3 a, Vec3 b, Shader sahder);

    BehaviorTree bhTree;
    
    AnimationClipSet *animationsSets;
    u32 numAnimationsSets;

    friend struct Editor;
};

#endif // _LEVEL_H_
