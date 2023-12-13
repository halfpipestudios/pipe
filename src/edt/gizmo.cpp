#include "gizmo.h"

#include "input.h"
#include "camera.h"
#include "game_window.h"
#include "graphics_manager.h"
#include "platform_manager.h"
#include "mgr/model_manager.h"

GizmoManager GizmoManager::gizmoManager;

void GizmoManager::Initialize(GameWindow *window, Camera *camera) {
    shader = GraphicsManager::Get()->CreateShaderVertex("./data/shaders/gizmoVert.hlsl",
                                                        "./data/shaders/gizmoFrag.hlsl");
    idFrameBufferW = 1280;
    idFrameBufferH = 720;
    idFrameBuffer = GraphicsManager::Get()->CreateFloatFrameBuffer(0, 0, idFrameBufferW, idFrameBufferH);

    pressedId = 0;

    this->camera = camera;
    this->window = window;
}

void GizmoManager::Terminate() {
    GraphicsManager::Get()->DestroyShader(shader);
    GraphicsManager::Get()->DestroyFrameBuffer(idFrameBuffer);
}

void GizmoManager::UpdateInput() {

#if 0
    Input *input = PlatformManager::Get()->GetInput();
    if(input->MouseIsPress(MOUSE_BUTTON_L)) {

        u32 w, h, sizeInBytes;
        u8 *buffer = nullptr;
        GraphicsManager::Get()->FrameBufferMap(idFrameBuffer, &w, &h, &sizeInBytes, &buffer);

        f32 *floatBuffer = (f32 *)buffer;
    
        u32 mouseX = 0;
        u32 mouseY = 0;

        pressedId = (u32)floatBuffer[(mouseX*4) + (mouseY*4*idFrameBufferW)];

        GraphicsManager::Get()->FrameBufferUnmap(idFrameBuffer);

    
    } else {
        pressedId = 0;
    }
#endif

}



Gizmo::Gizmo() {
    static u32 autoId = 0;
    id = ++autoId;
}

void Gizmo::Initialize(char *modelName, Vec3 color) {
    this->model = ModelManager::Get()->GetAsset(modelName);
    this->color = color;
    this->transform.pos   = Vec3(0,0,0);
    this->transform.rot   = Vec3(0,0,0);
    this->transform.scale = Vec3(1,1,1);
    this->visible = true;
}

void Gizmo::Render() {
    if(visible) {
        
        GameWindow *window = GizmoManager::Get()->window;

        GraphicsManager::Get()->SetWorldMatrix(transform.GetWorldMatrix());
        
        // --------------------------------------------------------------- 
        CBGizmo *buffer = &GraphicsManager::Get()->cpuGizmoBuffer;
        ConstBuffer constBufferHandle = GraphicsManager::Get()->gpuGizmoBuffer;
        buffer->color = color;
        GraphicsManager::Get()->UpdateConstBuffer(constBufferHandle, buffer);
        RenderModel(model, color, window->gameFrameBuffer, window->gizmoShader);
        
        // --------------------------------------------------------------- 
        // TODO: Update const buffer for render gizmo id
        
        RenderModel(model, color, GizmoManager::Get()->idFrameBuffer, GizmoManager::Get()->shader);
    }
}

void Gizmo::Update() {
    lastPress = press;
    press = GizmoManager::Get()->pressedId == id;
}

bool Gizmo::IsPress() {
    return press;
}

bool Gizmo::WasPress() {
    return !lastPress && press;
}

bool Gizmo::WasRelease() {
    return lastPress && !press;
}

void Gizmo::SetTransform(TransformCMP transform) {
    
    Camera *camera = GizmoManager::Get()->camera;

    Vec3 p = transform.pos;
    Vec3 n = camera->front.Normalized();
    Vec3 o = camera->pos + n * 6;
   
    Vec3 v = transform.pos - camera->pos;

    f32 behindTest = v.Dot(n);

    if(behindTest >= 0) {

        visible = true;

        f32 t = 0;
        if(ABS(v.Dot(n)) > 0.01f)  {
            t = (o.Dot(n) - p.Dot(n)) / v.Dot(n);
        }

        transform.rot = Vec3(0,0,0);
        transform.scale = Vec3(1,1,1);
        transform.pos = p + v * t; 

    } else {
        visible = false;
    }

    this->transform = transform;
}

void Gizmo::RenderModel(Handle handle, Vec3 color, FrameBuffer frameBufferHandle, Shader shader) {
    
    GraphicsManager::Get()->BindFrameBuffer(frameBufferHandle);

    Model *model = ModelManager::Get()->Dereference(handle);

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;
        if(mesh->indexBuffer) {
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, shader);
        } else {
            GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, shader);
        }
    }
}
