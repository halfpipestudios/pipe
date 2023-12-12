#ifndef _COMPONENTS_WINDOW_H_
#define _COMPONENTS_WINDOW_H_

#include "editor_window.h"

struct Entity_;

struct ComponentsWindow : EditorWindow {
    void Update(Editor *editor, f32 dt) override;
    void Render(Editor *editor) override;

    void UpdateTransformComponent(Entity_ *entity);
    void UpdatePhysicsComponent(Entity_ *entity);
    void UpdateCollisionComponent(Entity_ *entity);
    void UpdateGraphicComponent(Entity_ *entity);
    void UpdateInputComponent(Entity_ *entity);
    void UpdatePlayerAnimationComponent(Entity_ *entity);
    void UpdateAIComponent(Entity_ *entity);
    void UpdateMovingPlatformComponent(Entity_ *entity);

    i32 current_x;
    i32 current_y;
};

#endif // _COMPONENTS_WINDOW_H_
