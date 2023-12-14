#ifndef _COMPONENTS_WINDOW_H_
#define _COMPONENTS_WINDOW_H_

#include "editor_window.h"
#include <data_structures.h>

struct ComponentsWindow : EditorWindow {
    void Update(Editor *editor, f32 dt) override;
    void Render(Editor *editor) override;

    void UpdateTransformComponent(Editor *editor, SlotmapKey entityKey);
    void UpdatePhysicsComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateCollisionComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateGraphicComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateInputComponent(Editor *editor, SlotmapKey entityKey);
    void UpdatePlayerAnimationComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateAIComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateMovingPlatformComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateTriggerComponent(Editor *editor, SlotmapKey entityKey);
    void UpdateGemComponent(Editor *editor, SlotmapKey entityKey);

    i32 current_x;
    i32 current_y;
};

#endif // _COMPONENTS_WINDOW_H_
