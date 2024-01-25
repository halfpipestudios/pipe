#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "common.h"
#include "allocators.h"
#include "map_importer.h"
#include "model_importer.h"
#include "camera.h"
#include "entity.h"
#include "serializer.h"
#include "tokenizer.h"

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
#include "sys/particle_sys.inl"
#include "sys/fire_spell_sys.inl"
#include "sys/levitation_spell_sys.inl"
#include "sys/door_sys.inl"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    
    TextureBuffer texture;
    
    f32 scale;

    void Initialize(char *filename);
    void Terminate();
    void Render();
};

#define MAX_LEVEL_MAP_NAME_SIZE 32

struct Level : Serializable {

    void Initialize(char *levelPath, Camera *camera);
    //void Initialize(char *mapFilePath, Camera *camera);
    void Terminate();

    void BeginFrame(f32 dt);
    void EndFrame(f32 dt);

    void Update(f32 dt);
    void Render();

    bool DeleteEntity(SlotmapKey entityKey);
    void DeleteEntitiesToRemove();

    void SetCamera(Camera *camera) { this->camera = camera; }
    
    void Deserialize(Tokenizer *t) override;
    void Serialize(Serializer *s) override;

    MemoryFrame memory; 

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
    ParticleSys<EntityManager> particleSys;
    FireSpellSys<EntityManager> fireSpellSys;
    LevitationSpellSys<EntityManager> levitationSpellSys;
    DoorSys<EntityManager> doorSys;

    Camera *camera;
    Map map;
    char mapName[MAX_LEVEL_MAP_NAME_SIZE];

    SlotmapKey heroKey;
    Array<SlotmapKey> entities;
    FrameArray<SlotmapKey> entitiesToRemove;
 
    BehaviorTree bhTree;

    f32 gameTime { 0 };

private:

    void DestroyEntityAndComponents(SlotmapKey entityKey);

};

#endif // _LEVEL_H_
