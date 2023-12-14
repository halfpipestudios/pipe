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
#include "sys/trigger_sys.inl"
#include "sys/gem_sys.inl"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;

    TextureBuffer texture;
    
    f32 scale;

    void Initialize(char *filename);
    void Terminate();
    void Render(Shader shader);
};

struct Level {

    void Initialize(char *mapFilePath, Shader mapShader, Shader animShader);
    void Terminate();

    void BeginFrame(f32 dt);
    void EndFrame(f32 dt);

    void Update(f32 dt);
    void Render(Shader mapShader);

    bool DeleteEntity(SlotmapKey entityKey);
    void DeleteEntitiesToRemove();

    MemoryFrame memory; 
    EntityManager em;

    GraphicsSys<EntityManager> graphicsSys;
    TransformSys<EntityManager> transformSys;
    PhysicsSys<EntityManager> physicsSys;
    AnimationSys<EntityManager> animationSys;
    InputSys<EntityManager> inputSys;
    CollisionSys<EntityManager> collisionSys;
    MovingPlatformSys<EntityManager> movingPlatformSys;
    AiSys<EntityManager> aiSys;
    TriggerSys<EntityManager> triggerSys;
    GemSys<EntityManager> gemSys;

    Camera camera;
    
    Map map;

    SlotmapKey heroKey;
    Array<SlotmapKey> entities;
    FrameArray<SlotmapKey> entitiesToRemove;
 
    BehaviorTree bhTree;

private:

    void DestroyEntityAndComponents(SlotmapKey entityKey);

};

#endif // _LEVEL_H_
