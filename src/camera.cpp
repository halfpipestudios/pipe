#include "camera.h"
#include "graphics_manager.h"
#include "level.h"
#include "input.h"
#include "geometry.h"
#include "edt/editor_window.h"
#include "edt/gizmo.h"

#include <float.h>
#include <stdio.h>

void Camera::Initialize(CameraType type) {
    maxDist = 5;
    dist = 5;
    this->type = type;
}

void Camera::SetViewMatrix() {
    switch(type) {
        case THIRD_PERSON_CAMERA: { GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos, target, Vec3(0, 1, 0))); } break;
        case FREE_CAMERA        : { GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos, pos + front, Vec3(0, 1, 0))); } break;
    }

}

void Camera::ProcessFreeCamera(EditorWindow *window, f32 deltaTime) {

    Input *input = PlatformManager::Get()->GetInput();

    i32 mouseWheelDelta = input->state[0].wheelDelta;
    

    if(window->MouseIsHot() && (GizmoManager::Get()->hot == 0)) {
        if(input->MouseJustPress(MOUSE_BUTTON_L) || input->MouseJustPress(MOUSE_BUTTON_R) || mouseWheelDelta != 0) {
            active = true;
        }
    }

    if((input->MouseJustUp(MOUSE_BUTTON_L) || input->MouseJustUp(MOUSE_BUTTON_R)) && active) {
        active = false;
    }


    if(GizmoManager::Get()->active != 0) {
        active = false;
    }

    if(active) {
        Window *osWindow = PlatformManager::Get()->GetWindow();
            
        if(input->MouseIsPress(MOUSE_BUTTON_L)) {
            f32 deltaX = ((f32)input->MouseX() - (f32)input->MouseLastX()) * 0.0015f;
            f32 deltaY = ((f32)input->MouseY() - (f32)input->MouseLastY()) * 0.0015f;

            rot.y -= deltaX;
            rot.x -= deltaY;

            if (rot.x >  (89.0f/180.0f) * (f32)PI)
                rot.x =  (89.0f/180.0f) * (f32)PI;
            if (rot.x < -(89.0f/180.0f) * (f32)PI)
                rot.x = -(89.0f/180.0f) * (f32)PI;
        }

        if(input->MouseIsPress(MOUSE_BUTTON_R)) {
            f32 deltaX = ((f32)input->MouseX() - (f32)input->MouseLastX()) * 0.015f;
            f32 deltaY = ((f32)input->MouseY() - (f32)input->MouseLastY()) * 0.015f;
            pos = pos + right *  deltaX;
            pos = pos + up    * -deltaY;
        }

        if(mouseWheelDelta != 0) {
            pos = pos + front * ((f32)mouseWheelDelta);
        }

        if(input->MouseIsPress(MOUSE_BUTTON_L) || input->MouseIsPress(MOUSE_BUTTON_R)) {
            i32 mouseSetPositionX = osWindow->GetPosX() + window->GetPosX() + (window->GetWidth()/2);     
            i32 mouseSetPositionY = osWindow->GetPosY() + window->GetPosY() + (window->GetHeight()/2);     

            PlatformManager::Get()->SetMousePosition(mouseSetPositionX, mouseSetPositionY);

            input->state[0].mouseX = window->GetPosX() + (window->GetWidth()/2);
            input->state[0].mouseY = window->GetPosY() + (window->GetHeight()/2);
            input->state[1].mouseX = window->GetPosX() + (window->GetWidth()/2);
            input->state[1].mouseY = window->GetPosY() + (window->GetHeight()/2);
            
            
        }

        front = {0, 0, 1};
        front = Mat4::TransformVector(Mat4::RotateX(rot.x), front);
        front = Mat4::TransformVector(Mat4::RotateY(rot.y), front);
        front.Normalize();

        right = Vec3(0, 1, 0).Cross(front).Normalized();
        up = front.Cross(right).Normalized();
    }

}

void Camera::ProcessThirdPersonCamera(Map *map, f32 deltaTime) {
    
    Input *input = PlatformManager::Get()->GetInput();


    if(input->MouseIsPress(MOUSE_BUTTON_L)) {
        f32 deltaX = ((f32)input->MouseX() - (f32)input->MouseLastX()) * 0.0015f;
        f32 deltaY = ((f32)input->MouseY() - (f32)input->MouseLastY()) * 0.0015f;
        rot.y -= deltaX;
        rot.x -= deltaY;
        // TODO: set mouse position
    } else {

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
    }

    f32 zoomSpeed = 0.25f;
    if(input->KeyIsPress(KEY_F) || input->JoystickIsPress(JOYSTICK_BUTTON_DOWN)) {
        maxDist = MIN(maxDist + zoomSpeed, 15.0f);

    }
    if(input->KeyIsPress(KEY_R) || input->JoystickIsPress(JOYSTICK_BUTTON_UP)) {
        maxDist = MAX(maxDist - zoomSpeed, 1.0f);
    }

    if (rot.x >  (89.0f/180.0f) * (f32)PI)
        rot.x =  (89.0f/180.0f) * (f32)PI;
    if (rot.x < -(89.0f/180.0f) * (f32)PI)
        rot.x = -(89.0f/180.0f) * (f32)PI;

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
    for(u32 i = 0; i < map->entities.count; ++i) {
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


void Camera::ProcessMovement(void *data, f32 deltaTime) {

    switch(type) {
        case THIRD_PERSON_CAMERA: { ProcessThirdPersonCamera((Map *)data, deltaTime); } break;
        case FREE_CAMERA        : { ProcessFreeCamera((EditorWindow *)data, deltaTime); } break;
    }

}


Vec3 Camera::GetWorldFront() {
    return right.Cross(Vec3(0, 1, 0)).Normalized();
}

Ray Camera::GetMouseRay(Mat4 proj, f32 w, f32 h, f32 mouseX, f32 mouseY) {
    
    Mat4 invView = Mat4::Inverse(GraphicsManager::Get()->cpuMatrices.view);
    Mat4 invProj = Mat4::Inverse(proj);

    Vec4 rayClip;
    rayClip.x = 2.0f * (mouseX / (w - 1)) - 1.0f;
    rayClip.y = 1.0f - 2.0f * (mouseY / (h - 1));
    rayClip.z = 1.0f;
    rayClip.w = 1.0f;
    Vec4 rayEye = invProj * rayClip;
    rayEye.z = 1.0f;
    rayEye.w =  0.0f;
    Vec4 rayWorld = invView * rayEye;
    rayWorld.Normalize();

    Ray ray;
    ray.o = pos;
    ray.d = {rayWorld.x, rayWorld.y, rayWorld.z};
    ray.d.Normalize();
    return ray;
}
