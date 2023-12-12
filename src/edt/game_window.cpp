#include "game_window.h"

#include "game.h"

#include "graphics_manager.h"
#include "mgr/model_manager.h"

#include "editor.h"
#include "entity.h"

#include "cmp/transform_cmp.h"

void GameWindow::Initialize(char *name, TGuiWindowFlags flags, EditorWindow *otherWindow, TGuiSplitDirection dir) {
    EditorWindow::Initialize(name, flags, otherWindow, dir);
    gameFrameBuffer = GraphicsManager::Get()->CreateFrameBuffer(0, 0, 1280, 720);
    gizmoShader = GraphicsManager::Get()->CreateShaderVertex("./data/shaders/gizmoVert.hlsl",
                                                       "./data/shaders/gizmoFrag.hlsl");

    transformGizmoX = ModelManager::Get()->GetAsset("transform.twm");
    transformGizmoY = ModelManager::Get()->GetAsset("transform.twm");
    transformGizmoZ = ModelManager::Get()->GetAsset("transform.twm");
}

void GameWindow::Terminate() {
    EditorWindow::Terminate();
    GraphicsManager::Get()->DestroyShader(gizmoShader);
    GraphicsManager::Get()->DestroyFrameBuffer(gameFrameBuffer);
}

void GameWindow::Update(Editor *editor, f32 dt) {
    if(editor->paused) {
        _tgui_label(window, "game: paused", 0x00ff00, 4, 4, TGUI_ID);
    } else {
        _tgui_label(window, "game: playing", 0x00ff00, 4, 4, TGUI_ID);
    }
    tgui_texture(window, (void *)GraphicsManager::Get()->FrameBufferGetTexture(gameFrameBuffer));
}

void GameWindow::Render(Editor *editor) {
    
    i32 tguiWindowW = tgui_window_width(window); 
    i32 tguiWindowH = tgui_window_height(window); 
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)tguiWindowW/(f32)tguiWindowH, 0.01f, 1000.0f));

    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_LINEAR);
    GraphicsManager::Get()->SetViewport(0, 0, 1280, 720);
    GraphicsManager::Get()->BindFrameBuffer(gameFrameBuffer);
    GraphicsManager::Get()->ClearColorBuffer(gameFrameBuffer, 0.1f, 0.05f, 0.4f);
    GraphicsManager::Get()->ClearDepthStencilBuffer(gameFrameBuffer);

    editor->game->Render();
    RenderEditorGizmos(editor);

    GraphicsManager::Get()->FlushFrameBuffer(gameFrameBuffer);
}

void GameWindow::RenderEditorGizmos(Editor *editor) {
    
    Entity_ *selectedEntity = editor->selectedEntity;

    if(!selectedEntity) return;

    GraphicsManager::Get()->SetDepthStencilState(false);

    TransformCMP transform = *selectedEntity->GetComponent<TransformCMP>();

    Level *level = &editor->game->level;
    Camera camera = level->camera;

    Vec3 p = transform.pos;
    Vec3 n = camera.front.Normalized();
    Vec3 o = camera.pos + n * 6;
   
    Vec3 v = transform.pos - camera.pos;

    f32 behindTest = v.Dot(n);

    if(behindTest >= 0) {

        f32 t = 0;
        if(ABS(v.Dot(n)) > 0.01f)  {
            t = (o.Dot(n) - p.Dot(n)) / v.Dot(n);
        }

        transform.rot = Vec3(0,0,0);
        transform.scale = Vec3(1,1,1);
        transform.pos = p + v * t; 

        GraphicsManager::Get()->SetWorldMatrix(transform.GetWorldMatrix());
        RenderModel(editor, transformGizmoX, Vec3(1,0,0));
        
        TransformCMP transform1 = transform;
        transform1.rot.z += (f32)TO_RAD(90);

        GraphicsManager::Get()->SetWorldMatrix(transform1.GetWorldMatrix());
        RenderModel(editor, transformGizmoY, Vec3(0,1,0));

        TransformCMP transform2 = transform;
        transform2.rot.y += (f32)TO_RAD(90);
        
        GraphicsManager::Get()->SetWorldMatrix(transform2.GetWorldMatrix());
        RenderModel(editor, transformGizmoZ, Vec3(0,0,1));

    }

    GraphicsManager::Get()->SetDepthStencilState(true);
}

void GameWindow::RenderModel(Editor *editor, Handle handle, Vec3 color) {
    CBGizmo *buffer = &GraphicsManager::Get()->cpuGizmoBuffer;
    ConstBuffer constBufferHandle = GraphicsManager::Get()->gpuGizmoBuffer;
    buffer->color = color;
    GraphicsManager::Get()->UpdateConstBuffer(constBufferHandle, buffer);

    Model *model = ModelManager::Get()->Dereference(handle);

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;
        if(mesh->indexBuffer) {
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, gizmoShader);
        } else {
            GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, gizmoShader);
        }
    }
}
