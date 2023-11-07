#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "math.h"

struct Input;

struct Camera {

    Vec3 pos;
    Vec3 lastPos;
    Vec2 rot;
    Vec3 front;
    Vec3 right;
    Vec3 up;
    f32 speed;

    f32 dist;
    f32 maxDist;

    Vec3 vel;
    Vec3 acc;
    bool grounded;

    void Initialize(Vec3 pos_, f32 speed_);
    void ProcessMovement(Input *input, f32 deltaTime);
    void SetViewMatrix();
};

#endif
