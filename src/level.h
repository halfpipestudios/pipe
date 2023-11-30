#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "common.h"
#include "allocators.h"
#include "map_importer.h"
#include "model_importer.h"
#include "camera.h"
#include "entity.h"

#include "behavior_tree.h"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    TextureBuffer texture;
    f32 scale;
};

struct Level {
    
    void Initialize(char *mapFilePath, Shader statShader, Shader animShader);
    void Terminate();

    void Update(f32 dt);
    void Render(Shader statShader, Shader animShader);

private:

    Camera camera;
    
    Map map;

    Entity *hero;
    Entity *orc;
    Entity *platformHor;
    Entity *platformVer0;
    Entity *platformVer1;
 
    Entity *entitiesEnd;
    Entity *entities;
    ObjectAllocator<Entity> entitiesAllocator;

    Entity *AddEntity();
    Entity *AddEntity(Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader);
    Entity *AddMovingPlatform(Vec3 scale, Vec3 a, Vec3 b, Shader sahder);

    BehaviorTree bhTree;
    
    AnimationClipSet *animationsSets;
    u32 numAnimationsSets;
};

#endif // _LEVEL_H_
