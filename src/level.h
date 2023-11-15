#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "common.h"
#include "allocators.h"
#include "gjk_collision.h"
#include "map_importer.h"
#include "model_importer.h"
#include "camera.h"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    TextureBuffer texture;
    f32 scale;
};

struct Transform {
    Vec3 pos;
    Vec3 rot;
    Vec3 scale;
    inline Mat4 GetWorldMatrix() { return Mat4::Translate(pos) * Mat4::Rotate(rot) * Mat4::Scale(scale); };
};

struct Physics {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;
};

enum EntityFlags {
    ENTITY_STATIC    = 1 << 0,
    ENTITY_DYNAMIC   = 1 << 1,
    
    ENTITY_GROUNDED  = 1 << 2,
    ENTITY_COLLIDING = 1 << 3,
};

struct Entity {

    void Initialize(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations);
    void Terminate();

    void Update(Map *map, f32 dt);
    void Render(Shader shader);

    void Move(Input *input, Camera camera);
    
    Entity *next;

    Transform transform;
    Physics physics;
    Physics lastPhysics;
    Cylinder collider;

    Vec3 velXZ;
private:
    u32 flags;

    Model model;
    AnimationSet animation;
    
    Mat4 *finalTransformMatrices;
    u32 numFinalTrasformMatrices;

    inline void AddFlag(EntityFlags flag) { flags |= flag; }
    inline void RemoveFlag(EntityFlags flag) { flags &= ~flag; }
    inline void ClearFlags() { flags = 0; };
    inline bool HaveFlag(EntityFlags flag) { return (flags & flag) != 0; }

};

struct Level {
    
    void Initialize(char *mapFilePath);
    void Terminate();

    void Update(f32 dt);
    void Render(Shader statShader, Shader animShader);

private:

    Camera camera;
    
    Map map;

    Entity *hero;
    Entity *orc;
    
    Entity *entities;
    ObjectAllocator<Entity> entitiesAllocator;

    Entity *AddEntity(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations);
};

#endif // _LEVEL_H_
