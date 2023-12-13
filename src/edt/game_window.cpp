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

    X.Initialize("transform.twm", Vec3(1, 0, 0));
    Y.Initialize("transform.twm", Vec3(0, 1, 0));
    Z.Initialize("transform.twm", Vec3(0, 0, 1));

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

    X.Update();
    Y.Update();
    Z.Update();
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
    
    if(editor->selectedEntity) {

        GraphicsManager::Get()->SetDepthStencilState(false);
        
        TransformCMP transform = *editor->game->level.em.GetComponent<TransformCMP>(*editor->selectedEntity);
        transform.rot   = Vec3(0, 0, 0);
        transform.scale = Vec3(2, 2, 2);
        
        X.SetTransform(transform);
        X.Render();
        
        TransformCMP transform1 = transform;
        transform1.rot.z += (f32)TO_RAD(90);
        Y.SetTransform(transform1);
        Y.Render();
        
        TransformCMP transform2 = transform;
        transform2.rot.y += (f32)TO_RAD(90);
        Z.SetTransform(transform2);
        Z.Render();

        GraphicsManager::Get()->SetDepthStencilState(true);
    }

    GraphicsManager::Get()->FlushFrameBuffer(gameFrameBuffer);
}
