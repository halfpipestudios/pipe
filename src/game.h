#ifndef _GAME_H_
#define _GAME_H_

#include "common.h"

#include "level.h"

struct Game {

    void Initialize();
    void Terminate();

    void BeginFrame(f32 dt);
    void EndFrame(f32 dt);

    void Update(f32 dt);

    void Render();

    Shader mapShader;
    Shader animShader;
    Shader statShader;

    // Fire particle system shaders
    Shader soFireShader;
    GeometryShader soFireGeoShader;
    Shader dwFireShader;
    GeometryShader dwFireGeoShader;
 
    // Rain particle system shaders
    Shader soRainShader;
    GeometryShader soRainGeoShader;
    Shader dwRainShader;
    GeometryShader dwRainGeoShader;

    Level level;
    Camera camera;

    ParticleSystem firePS;
    ParticleSystem rainPS;

    f32 gameTime { 0.0f };
};

#endif // _GAME_H_
