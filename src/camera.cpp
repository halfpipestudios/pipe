#include "camera.h"
#include "graphics_manager.h"
#include "level.h"
#include "input.h"
#include "geometry.h"

#include <float.h>
#include <stdio.h>

void Camera::Initialize() {
    maxDist = 5;
    dist = 5; 
}

void Camera::SetViewMatrix() {
    GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos, target, Vec3(0, 1, 0)));
}

void Camera::ProcessMovement(Input *input, Map *map, f32 deltaTime) {
    // TODO: mouse
    if(input->KeyIsPress(KEY_LEFT)) {
        rot.y += 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_RIGHT)) {
        rot.y -= 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_UP)) {
        rot.x += 2.5f * deltaTime;
    }
    if(input->KeyIsPress(KEY_DOWN)) {
        rot.x -= 2.5f * deltaTime;
    }


    rot.y -= input->state[0].rightStickX * 2.5f * deltaTime;
    rot.x += input->state[0].rightStickY * 2.5f * deltaTime;

    f32 zoomSpeed = 0.25f;
    if(input->KeyIsPress(KEY_F) || input->JoystickIsPress(JOYSTICK_BUTTON_DOWN)) {
        maxDist = MIN(maxDist + zoomSpeed, 15.0f);

    }
    if(input->KeyIsPress(KEY_R) || input->JoystickIsPress(JOYSTICK_BUTTON_UP)) {
        maxDist = MAX(maxDist - zoomSpeed, 1.0f);
    }

    if (rot.x >  (89.0f/180.0f) * PI)
        rot.x =  (89.0f/180.0f) * PI;
    if (rot.x < -(89.0f/180.0f) * PI)
        rot.x = -(89.0f/180.0f) * PI;

    front = {0, 0, 1};
    front = Mat4::TransformVector(Mat4::RotateX(rot.x), front);
    front = Mat4::TransformVector(Mat4::RotateY(rot.y), front);
    front.Normalize();

    right = Vec3(0, 1, 0).Cross(front).Normalized();
    up = front.Cross(right).Normalized();


    Segment cameraSegment;
    cameraSegment.a = target;
    cameraSegment.b = target - (front * maxDist);

    f32 tMin = FLT_MAX;
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *entity = &map->entities.data[i];
        f32 t = -1.0f;
        if(cameraSegment.HitEntity(entity, &t)) {
            if(t < tMin) {
                tMin = t;
            }
        }
    }

    dist = maxDist;
    if(tMin > 0.0f && tMin <= 1.0f) {
        dist = MIN((maxDist-0.1f) * tMin, maxDist);
    }

    pos = target - (front * dist);
}


Vec3 Camera::GetWorldFront() {
    return right.Cross(Vec3(0, 1, 0)).Normalized();
}
