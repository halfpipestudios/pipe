#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "algebra.h"
#include "geometry.h"

struct Map;
struct Input;
    
enum CameraType {
    THIRD_PERSON_CAMERA,
    FIRST_PERSON_CAMERA,
    FREE_CAMERA
};

struct Camera {

    CameraType type;

    Vec3 pos;
    Vec3 target;

    f32 dist;
    f32 maxDist;
    
    Vec2 rot;
    Vec3 front;
    Vec3 right;
    Vec3 up;

    void Initialize(CameraType type);
    void ProcessMovement(Map *map, f32 deltaTime);
    void SetTarget(Vec3 target) { this->target = target; }
    void SetViewMatrix();
    Vec3 GetWorldFront();

    Ray GetMouseRay(Mat4 proj, f32 w, f32 h, f32 mouseX, f32 mouseY);

private:
    void ProcessThirdPersonCamera(Map *map, f32 deltaTime);
    void ProcessFreeCamera(Map *map, f32 deltaTime);

};

#endif
