#ifndef _GIZMO_H_
#define _GIZMO_H_

#include "common.h"
#include "graphics.h"
#include "asset_manager.h"

#include "cmp/base_cmp.h"
#include "cmp/transform_cmp.h"

struct Camera;
struct GameWindow;
struct TGuiWindow;

struct GizmoManager {
    VShader vShader;
    FShader fShader;
    
    u32 active;
    u32 hot;

    u32 idFrameBufferW;
    u32 idFrameBufferH;

    FrameBuffer idWriteFrameBuffer;
    FrameBuffer idReadFrameBuffer;

    GameWindow *window;

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
    TransformCMP renderTransform;
    
    Gizmo();
    
    void Initialize(char *modelName, Vec3 color);
    void Render();
    
    bool IsHot();
    bool IsActive();

    void SetHot(bool state);
    void SetActive(bool state);

    void SetTransform(Camera *camera, TransformCMP transform);
    void RenderModel(Handle handle, Vec3 color, FrameBuffer frameBufferHandle, VShader vShader, FShader fShader);

private:

};

#endif // _GIZMO_H_
