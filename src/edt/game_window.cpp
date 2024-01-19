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
    gizmoVShader = GraphicsManager::Get()->CreateVShader("./data/shaders/gizmoVert.hlsl");
    gizmoFShader = GraphicsManager::Get()->CreateFShader("./data/shaders/gizmoFrag.hlsl");

    X.Initialize("transform.twm", Vec3(0.8f, 0, 0));
    Y.Initialize("transform.twm", Vec3(0, 0.8f, 0));
    Z.Initialize("transform.twm", Vec3(0, 0, 0.8f));

}

void GameWindow::Terminate() {
    EditorWindow::Terminate();
    GraphicsManager::Get()->DestroyVShader(gizmoVShader);
    GraphicsManager::Get()->DestroyFShader(gizmoFShader);
    GraphicsManager::Get()->DestroyFrameBuffer(gameFrameBuffer);
}

void GameWindow::UpdateTransformGizmos() {
    Input *input = PlatformManager::Get()->GetInput();
    if(X.IsActive()) {
        if(input->MouseJustUp(MOUSE_BUTTON_L)) {
            X.SetActive(false);
        }
    } else if(X.IsHot() ) {
        if(input->MouseJustPress(MOUSE_BUTTON_L)) {
            X.SetActive(true);
            FirstClick = true;
        }
    }
    if(Y.IsActive()) {
        if(input->MouseJustUp(MOUSE_BUTTON_L)) {
            Y.SetActive(false);
        }
    } else if(Y.IsHot() ) {
        if(input->MouseJustPress(MOUSE_BUTTON_L)) {
            Y.SetActive(true);
            FirstClick = true;
        }
    }
    if(Z.IsActive()) {
        if(input->MouseJustUp(MOUSE_BUTTON_L)) {
            Z.SetActive(false);
        }
    } else if(Z.IsHot() ) {
        if(input->MouseJustPress(MOUSE_BUTTON_L)) {
            Z.SetActive(true);
            FirstClick = true;
        }
    }
}

Vec3 GameWindow::CalculateNewTransformPosition(Camera *camera, Gizmo *gizmo, Vec3 n0, Vec3 n1) {

    Mat4 proj = Mat4::Perspective(60, (f32)GetWidth()/(f32)GetHeight(), 0.01f, 1000.0f);

    i32 mouseX = GetMouseX();
    i32 mouseY = GetMouseY();
    i32 width  = GetWidth();
    i32 height = GetHeight();

    Ray mouse = camera->GetMouseRay(proj, (f32)width, (f32)height, (f32)mouseX, (f32)mouseY);

    Vec3 normal = {};
    if(ABS(mouse.d.Dot(n0)) > ABS(mouse.d.Dot(n1))) {
        normal = n0;
    } else {
        normal = n1;
    }

    Vec3 mousePosition = mouse.IntersectPlane(gizmo->renderTransform.pos, normal);
    
    if(FirstClick) {
        Offset = mousePosition - gizmo->renderTransform.pos;
        FirstClick = false;
    }
    
    Vec3 newRenderPos = mousePosition - Offset;

    Ray movementRay = {};
    movementRay.o = camera->pos;
    movementRay.d = (newRenderPos - camera->pos).Normalized();

    return movementRay.IntersectPlane(gizmo->transform.pos, normal);

}

void GameWindow::Update(Editor *editor, f32 dt) {
    
    if(editor->paused) dt = 0;

    Camera *camera = &editor->camera;

    if(editor->paused) {
        _tgui_label(window, "game: paused", 0x00ff00, 4, 4, TGUI_ID);
    } else {
        _tgui_label(window, "game: playing", 0x00ff00, 4, 4, TGUI_ID);
    }
    tgui_texture(window, (void *)GraphicsManager::Get()->FrameBufferGetTexture(gameFrameBuffer));
    
    UpdateTransformGizmos();

    Mat4 proj = Mat4::Perspective(60, (f32)GetWidth()/(f32)GetHeight(), 0.01f, 1000.0f);

    if(X.IsActive()) {
        
        ASSERT(editor->selectedEntity);

        Vec3 normal = {};
        Vec3 n0 = Vec3(0, 0, 1);
        Vec3 n1 = Vec3(0, 1, 0);
        
        TransformCMP *transform = EntityManager::Get()->GetComponent<TransformCMP>(*editor->selectedEntity);
        transform->pos.x = CalculateNewTransformPosition(camera, &X, n0, n1).x;

        PhysicsCMP *physicCmp = EntityManager::Get()->GetComponent<PhysicsCMP>(*editor->selectedEntity);
        if(physicCmp) {
            physicCmp->physics.pos = transform->pos;
        }

    }

    if(Y.IsActive()) {

        ASSERT(editor->selectedEntity);

        TransformCMP *transform = EntityManager::Get()->GetComponent<TransformCMP>(*editor->selectedEntity);
        Vec3 n = (transform->pos - camera->pos);
        transform->pos.y = CalculateNewTransformPosition(camera, &X, n, n).y;

        PhysicsCMP *physicCmp = EntityManager::Get()->GetComponent<PhysicsCMP>(*editor->selectedEntity);
        if(physicCmp) {
            physicCmp->physics.pos = transform->pos;
        }

    }
    
    if(Z.IsActive()) {

        ASSERT(editor->selectedEntity);

        Vec3 normal = {};
        Vec3 n0 = Vec3(1, 0, 0);
        Vec3 n1 = Vec3(0, 1, 0);

        TransformCMP *transform = EntityManager::Get()->GetComponent<TransformCMP>(*editor->selectedEntity);
        transform->pos.z = CalculateNewTransformPosition(camera, &X, n0, n1).z;

        PhysicsCMP *physicCmp = EntityManager::Get()->GetComponent<PhysicsCMP>(*editor->selectedEntity);
        if(physicCmp) {
            physicCmp->physics.pos = transform->pos;
        }
    }

    editor->level->Update(dt);
}

void GameWindow::Render(Editor *editor) {
    
    i32 tguiWindowW = tgui_window_width(window); 
    i32 tguiWindowH = tgui_window_height(window); 
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)GetWidth()/(f32)GetHeight(), 0.01f, 1000.0f));

    GraphicsManager::Get()->SetSamplerState(SAMPLER_STATE_LINEAR);
    GraphicsManager::Get()->SetViewport(0, 0, 1280, 720);
    GraphicsManager::Get()->BindFrameBuffer(gameFrameBuffer);
    GraphicsManager::Get()->ClearColorBuffer(gameFrameBuffer, 0.1f, 0.05f, 0.4f);
    GraphicsManager::Get()->ClearDepthStencilBuffer(gameFrameBuffer);

    editor->level->Render();
    
    if(editor->selectedEntity) {

        if(X.IsHot()) {
            X.color = Vec3(1,0.2f,0.2f);
        } else {
            X.color = Vec3(0.8f,0.1f,0.1f);
        } 
        if(Y.IsHot()) {
            Y.color = Vec3(0.2f,1,0.2f);
        } else {
            Y.color = Vec3(0.1f,0.8f,0.1f);
        } 
        if(Z.IsHot()) {
            Z.color = Vec3(0.2f,0.2f,1);
        } else {
            Z.color = Vec3(0.1f,0.1f,0.8f);
        } 

        GraphicsManager::Get()->SetDepthStencilState(false);
        
        ASSERT(editor->selectedEntity);
        TransformCMP transform = *EntityManager::Get()->GetComponent<TransformCMP>(*editor->selectedEntity);
        transform.rot   = Vec3(0, (f32)TO_RAD(180), 0);
        transform.scale = Vec3(1.2f, 1.2f, 1.2f);
        
        X.SetTransform(&editor->camera, transform);
        X.Render();
         
        TransformCMP transform1 = transform;
        transform1.rot.z += (f32)TO_RAD(90);
        Y.SetTransform(&editor->camera, transform1);
        Y.Render();
        
        TransformCMP transform2 = transform;
        transform2.rot.y += (f32)TO_RAD(90);
        Z.SetTransform(&editor->camera, transform2);
        Z.Render();
        
        GraphicsManager::Get()->SetDepthStencilState(true);
    }

    GraphicsManager::Get()->FlushFrameBuffer(gameFrameBuffer);
}
