#ifndef _GAME_H_
#define _GAME_H_

#include "common.h"
#include "camera.h"

#include "level.h"

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

    Level level;

#if 0
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
#endif
};

#endif // _GAME_H_
