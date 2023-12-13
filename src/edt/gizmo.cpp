#include "gizmo.h"

#include "input.h"
#include "camera.h"
#include "game_window.h"
#include "graphics_manager.h"
#include "platform_manager.h"
#include "mgr/model_manager.h"

GizmoManager GizmoManager::gizmoManager;

void GizmoManager::Initialize(GameWindow *window, Camera *camera) {
    shader = GraphicsManager::Get()->CreateShaderVertex("./data/shaders/pickingVert.hlsl",
                                                        "./data/shaders/pickingFrag.hlsl");
    idFrameBufferW = 1280;
    idFrameBufferH = 720;
    
    idWriteFrameBuffer = GraphicsManager::Get()->CreateWriteFrameBuffer(0, 0, idFrameBufferW, idFrameBufferH);
    idReadFrameBuffer  = GraphicsManager::Get()->CreateReadFrameBuffer(0, 0, idFrameBufferW, idFrameBufferH);

    pressedId = 0;

    this->camera = camera;
    this->window = window;
}

void GizmoManager::Terminate() {
    GraphicsManager::Get()->DestroyShader(shader);
    GraphicsManager::Get()->DestroyFrameBuffer(idWriteFrameBuffer);
    GraphicsManager::Get()->DestroyFrameBuffer(idReadFrameBuffer);
}

void GizmoManager::UpdateInput() {

    Input *input = PlatformManager::Get()->GetInput();

    i32 w_w = tgui_window_width(window->window);
    i32 w_h = tgui_window_height(window->window);
    TGuiWindow *w = tgui_window_get_from_handle(window->window);
        
    i32 mouseX = CLAMP(input->state[0].mouseX - w->dim.min_x, 0, w_w-1);
    i32 mouseY = CLAMP(input->state[0].mouseY - w->dim.min_y, 0, w_h-1);
    
    if(input->MouseIsPress(MOUSE_BUTTON_L)) {
        
        mouseX = (i32)(((f32)mouseX / ((f32)w_w - 1)) * (f32)(idFrameBufferW - 1));
        mouseY = (i32)(((f32)mouseY / ((f32)w_h - 1)) * (f32)(idFrameBufferH - 1));
        
        GraphicsManager::Get()->CopyFrameBuffer(idReadFrameBuffer, idWriteFrameBuffer);

        u32 w, h, sizeInBytes;
        u8 *buffer = nullptr;
        GraphicsManager::Get()->FrameBufferMap(idReadFrameBuffer, &w, &h, &sizeInBytes, &buffer);

        Vec4 *floatBuffer = (Vec4 *)buffer;

        Vec4 pressedId = floatBuffer[mouseX + mouseY*idFrameBufferW];
        printf("x:%f, y:%f, z:%f, w:%f\n", pressedId.x, pressedId.y, pressedId.z, pressedId.w);

        GraphicsManager::Get()->FrameBufferUnmap(idReadFrameBuffer);

    
    } else {
        pressedId = 0;
    }

    GraphicsManager::Get()->ClearColorBuffer(idWriteFrameBuffer, 0, 0, 0);

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
         
        {
            CBGizmo *buffer = &GraphicsManager::Get()->cpuGizmoBuffer;
            ConstBuffer constBufferHandle = GraphicsManager::Get()->gpuGizmoBuffer;
            buffer->color = color;
            GraphicsManager::Get()->UpdateConstBuffer(constBufferHandle, buffer);
            RenderModel(model, color, window->gameFrameBuffer, window->gizmoShader);
        }
       
        { 
            CBIndex *buffer = &GraphicsManager::Get()->cpuIndexBuffer;
            ConstBuffer constBufferHandle = GraphicsManager::Get()->gpuIndexBuffer;
            buffer->id = (f32)id;
            GraphicsManager::Get()->UpdateConstBuffer(constBufferHandle, buffer);
            RenderModel(model, color, GizmoManager::Get()->idWriteFrameBuffer, GizmoManager::Get()->shader);
        }

        GraphicsManager::Get()->BindFrameBuffer(window->gameFrameBuffer);
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
