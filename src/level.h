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

    void Initialize(char *filename);
    void Terminate();
    void Render(Shader shader);
};

struct Level {

    void Initialize(char *mapFilePath, Shader mapShader, Shader animShader);
    void Terminate();

    void Update(f32 dt);
    void Render(Shader mapShader);

private:
    
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

    Camera camera;
    
    Map map;

    Entity_ *hero;
    Entity_ *orc;
    Entity_ *orc1;
    Entity_ *platformHor;
    Entity_ *platformVer0;
    Entity_ *platformVer1;
 
    BehaviorTree bhTree;
    
    friend struct Editor;
};

#endif // _LEVEL_H_
