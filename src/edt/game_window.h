#ifndef _GAME_WINDOW_H_
#define _GAME_WINDOW_H_

#include "algebra.h"
#include "graphics.h"

#include "editor_window.h"
#include "asset_manager.h"

struct GameWindow : EditorWindow {

    void Initialize(char *name, TGuiWindowFlags flags, EditorWindow *otherWindow, TGuiSplitDirection dir) override;
    void Terminate() override;

    void Update(Editor *editor, f32 dt) override;
    void Render(Editor *editor) override;

    void RenderEditorGizmos(Editor *editor);
    void RenderModel(Editor *editor, Handle handle, Vec3 color);

    Handle transformGizmoX;
    Handle transformGizmoY;
    Handle transformGizmoZ;

    Shader gizmoShader;
    FrameBuffer gameFrameBuffer;

};

#endif //  _GAME_WINDOW_H_
