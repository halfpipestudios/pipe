#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "algebra.h"
#include "geometry.h"
#include "serializer.h"
#include "tokenizer.h"

struct Map;
struct EditorWindow;
struct Input;

    
enum CameraType {
    THIRD_PERSON_CAMERA,
    FIRST_PERSON_CAMERA,
    FREE_CAMERA
};

struct Camera : Serializable {

    CameraType type;

    Vec3 pos;
    Vec3 target;

    f32 dist;
    f32 maxDist;
    
    Vec2 rot;
    Vec3 front {0, 0, 1};
    Vec3 right;
    Vec3 up;

    bool rotActive { false };
    bool panActive { false };
    i32 mouseClickX { 0 };
    i32 mouseClickY { 0 };

    void Initialize(CameraType type);
    void ProcessMovement(void *data, f32 deltaTime);
    void SetTarget(Vec3 target) { this->target = target; }
    void SetViewMatrix();
    Vec3 GetWorldFront();

    Ray GetMouseRay(Mat4 proj, f32 w, f32 h, f32 mouseX, f32 mouseY);

    void Serialize(Serializer *s) override;
    void Deserialize(Tokenizer *t) override;

private:
    void ProcessThirdPersonCamera(Map *map, f32 deltaTime);
    void ProcessFreeCamera(EditorWindow *window, f32 deltaTime);

    void ProcessPan(Input *input);
    void ProcessRot(Input *input);
    void ProcessCommon(EditorWindow *window, Input *input);
    void UpdateCameraVectors();
    void RestoreMouseToLastClick(Input *input);

};

#endif
