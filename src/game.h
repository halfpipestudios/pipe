#ifndef _GAME_H_
#define _GAME_H_

#include "common.h"
#include "camera.h"
#include "map_importer.h"
#include "model_importer.h"
#include "gjk_collision.h"

struct Map {
    MapImporter::ConvexHullArray covexHulls;
    MapImporter::EntityArray entities;
    VertexBuffer vertexBuffer;
    TextureBuffer texture;
    f32 scale;
};

struct Game {

    void Initialize();
    void Terminate();

    void Update(f32 dt);

    void FixUpdate(f32 dt);
    void PostUpdate(f32 t);

    void Render();

private:

    Camera camera;
    
    Shader statShader;
    Shader animShader;
    Map map;

    Model orc;
    AnimationSet orcAnimation;
    Cylinder orcCollider;
    Mat4 *orcFinalTransformMatrices;
    u32 orcNumFinalTrasformMatrices;
    
    Model hero;
    AnimationSet heroAnimation;
    Cylinder heroCollider;
    Mat4 *heroFinalTransformMatrices;
    u32 heroNumFinalTrasformMatrices;


    u32 colorC;
};

#endif // _GAME_H_
