#ifndef _GIZMO_H_
#define _GIZMO_H_

#include "common.h"
#include "graphics.h"
#include "asset_manager.h"

#include "cmp/base_cmp.h"
#include "cmp/transform_cmp.h"

struct Camera;
struct GameWindow;

struct GizmoManager {
    Shader shader;
    u32 pressedId;

    u32 idFrameBufferW;
    u32 idFrameBufferH;

    FrameBuffer idWriteFrameBuffer;
    FrameBuffer idReadFrameBuffer;

    GameWindow *window;
    Camera *camera;

    void Initialize(GameWindow *window, Camera *camera);
    void Terminate();
    void UpdateInput();
    
    inline static GizmoManager *Get() { return &gizmoManager; }
    static GizmoManager gizmoManager;
};

struct Gizmo {
    
    u32 id;
    Handle model;
    Vec3 color;
    bool visible;

    TransformCMP transform;
    
    Gizmo();
    
    void Initialize(char *modelName, Vec3 color);
    void Render();
    void Update();

    bool IsPress();
    bool WasPress();
    bool WasRelease();

    void SetTransform(TransformCMP transform);
    void RenderModel(Handle handle, Vec3 color, FrameBuffer frameBufferHandle, Shader shader);

private:
    
    bool press;
    bool lastPress;

};

#endif // _GIZMO_H_
