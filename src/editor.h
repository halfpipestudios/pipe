#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "common.h"
#include "graphics.h"
#include <tgui.h>

#include "mgr/model_manager.h"

struct Game;
struct Level;
struct Entity_;

struct Editor {
    
    void Initialize(Game *game);
    void Terminate();
    
    void Update(f32 dt);
    void Render();

    void RenderEditorGizmos();
    void RenderModel(Handle handle, Vec3 color);
    
    Game *game;
    Entity_ *selectedEntity;
    bool paused;

    TGuiGfxBackend tguiBackend;

    TGuiWindowHandle gameWindow;
    TGuiWindowHandle toolWindow;
    TGuiWindowHandle compWindow;
    TGuiWindowHandle entiWindow;

    FrameBuffer gameFrameBuffer;

    Handle transformGizmoX;
    Handle transformGizmoY;
    Handle transformGizmoZ;

    Shader gizmoShader;

};

#endif // _EDITOR_H_
