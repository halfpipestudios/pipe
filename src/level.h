#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "common.h"
#include "allocators.h"
#include "gjk_collision.h"
#include "map_importer.h"
#include "model_importer.h"
#include "camera.h"
#include "entity.h"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    TextureBuffer texture;
    f32 scale;
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
