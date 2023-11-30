#ifndef _GAME_H_
#define _GAME_H_

#include "common.h"

#include "level.h"

struct Game {

    void Initialize();
    void Terminate();

    void Update(f32 dt);

    void FixUpdate(f32 dt);
    void PostUpdate(f32 t);

    void Render();

private:
 
    Shader mapShader;
    Shader animShader;
    Shader statShader;

    Level level;

};

#endif // _GAME_H_
