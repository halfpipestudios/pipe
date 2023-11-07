#include "camera.h"
#include "graphics_manager.h"
#include "input.h"

void Camera::Initialize(Vec3 pos_, f32 speed_) {
    pos = pos_;
    speed = speed_;
    vel = Vec3();
    acc = Vec3();
    grounded =  false;
    maxDist = 5;
    dist = 5; 
}


void Camera::SetViewMatrix() {
    Vec3 up  = Vec3(0, 1,  0);
    GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(pos - front * dist, pos, Vec3(0, 1, 0)));
}

void Camera::ProcessMovement(Input *input, f32 deltaTime)
{
    front = {0, 0, 1};
    front = Mat4::TransformVector(Mat4::RotateX(rot.x), front);
    front = Mat4::TransformVector(Mat4::RotateY(rot.y), front);
    front.Normalize();

    right = Vec3(0, 1, 0).Cross(front).Normalized();
    up = front.Cross(right).Normalized();

    Vec3 worldFront = right.Cross(Vec3(0, 1, 0)).Normalized();

    acc = {};
    if(input->KeyIsPress(KEY_W)) {
        acc += worldFront;
    }
    if(input->KeyIsPress(KEY_S)) {
        acc -= worldFront;
    }
    if(input->KeyIsPress(KEY_A)) {
        acc -= right;
    }
    if(input->KeyIsPress(KEY_D)) {
        acc += right;
    }

    f32 zoomSpeed = 0.25f;
    if(input->KeyIsPress(KEY_F)) {
        maxDist = MIN(maxDist + zoomSpeed, 15.0f);

    }
    if(input->KeyIsPress(KEY_R)) {
        maxDist = MAX(maxDist - zoomSpeed, 1.0f);
    }

    if(input->KeyJustPress(KEY_SPACE) && grounded) {
        Vec3 jumpForce = Vec3(0, 40, 0);
        vel += jumpForce;
    }

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

    if (rot.x >  (89.0f/180.0f) * PI)
        rot.x =  (89.0f/180.0f) * PI;
    if (rot.x < -(89.0f/180.0f) * PI)
        rot.x = -(89.0f/180.0f) * PI;

    if(acc.LenSq() > 0.0f) acc.Normalize();

    acc *= speed;

    if(!grounded)
        acc += Vec3(0, -9.8 * 5, 0);

    vel += acc * deltaTime;

    f32 dammping = powf(0.001f, deltaTime);
    vel = vel * dammping;

    lastPos = pos;
    pos += vel * deltaTime;
}
