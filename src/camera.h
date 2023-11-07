#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "math.h"

struct Map;
struct Input;

struct Camera {

    Vec3 pos;
    Vec3 target;

    f32 dist;
    f32 maxDist;
    
    Vec2 rot;
    Vec3 front;
    Vec3 right;
    Vec3 up;

    void Initialize();
    void ProcessMovement(Input *input, Map *map, f32 deltaTime);
    void SetViewMatrix();
    Vec3 GetWorldFront();
};

#endif
