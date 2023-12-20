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

    Shader soShader;
    GeometryShader soGeoShader;

    Shader drawShader;
    GeometryShader drawGeoShader;

    Level level;
    Camera camera;

    ParticleSystem particleSystem;

    f32 gameTime { 0.0f };
};

#endif // _GAME_H_
