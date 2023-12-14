#include "components_window.h"

#include "editor.h"
#include "entity.h"

#include "cmp/transform_cmp.h"
#include "cmp/graphics_cmp.h"
#include "cmp/physics_cmp.h"
#include "cmp/animation_cmp.h"
#include "cmp/input_cmp.h"
#include "cmp/collision_cmp.h"
#include "cmp/moving_platform_cmp.h"
#include "cmp/ai_cmp.h"
#include "cmp/trigger_cmp.h"
#include "cmp/gem_cmp.h"

#include "game.h"

void ComponentsWindow::Update(Editor *editor, f32 dt) {
    
    current_x = 10;
    current_y = 2;

    if(editor->selectedEntity == nullptr) {
        _tgui_label(window, "There is no entity selected!", 0x222222, 10, 10, TGUI_ID);
        return;
    }

    Entity_ *selectedEntity = editor->game->level.em.GetEntity(*editor->selectedEntity);

    if(selectedEntity) {

        for(u32 i = 0; i < selectedEntity->componentsIds.size; ++i) {
            u32 id = selectedEntity->componentsIds[i];
            if(id == TransformCMP::GetID()) {
                UpdateTransformComponent(editor, *editor->selectedEntity);
            } else if(id == PhysicsCMP::GetID()) {
                UpdatePhysicsComponent(editor, *editor->selectedEntity);
            } else if(id == CollisionCMP::GetID()) {
                UpdateCollisionComponent(editor, *editor->selectedEntity);
            } else if(id == GraphicsCMP::GetID()) {
                UpdateGraphicComponent(editor, *editor->selectedEntity);
            } else if(id == InputCMP::GetID()) {
                UpdateInputComponent(editor, *editor->selectedEntity);
            } else if(id == AnimationCMP::GetID()) {
                UpdatePlayerAnimationComponent(editor, *editor->selectedEntity);
            } else if(id == MovingPlatformCMP::GetID()) {
                UpdateMovingPlatformComponent(editor, *editor->selectedEntity);
            } else if(id == AiCMP::GetID()) {
                UpdateAIComponent(editor, *editor->selectedEntity);
            } else if (id == TriggerCMP::GetID()){
                UpdateTriggerComponent(editor, *editor->selectedEntity);
            } else if (id == GemCMP::GetID()){
                UpdateGemComponent(editor, *editor->selectedEntity);
            } else {
                char *compName = "Unknown Component";
                _tgui_label(window, compName, 0x222222, current_x, current_y, compName);
                current_y += 20;
            }
        }

    } else {
        _tgui_label(window, "There is no entity selected!", 0x222222, 10, 10, TGUI_ID);
    }

}

void ComponentsWindow::Render(Editor *editor) {
}


void ComponentsWindow::UpdateTransformComponent(Editor *editor, SlotmapKey entityKey) {

    TransformCMP *transform = editor->game->level.em.GetComponent<TransformCMP>(entityKey);
    ASSERT(transform);
    
    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = current_x + 10;
    u32 x = label_x + 40;
    
    if(_tgui_separator(window, "Transform Component", current_y, true, TGUI_ID)) {
        current_y += h + 10;

        _tgui_label(window, "p =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &transform->pos.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->pos.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->pos.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "r =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &transform->rot.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->rot.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->rot.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "S =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &transform->scale.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->scale.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &transform->scale.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);
    }

    current_y += h;
}

void ComponentsWindow::UpdatePhysicsComponent(Editor *editor, SlotmapKey entityKey) {
    PhysicsCMP *physicComp = editor->game->level.em.GetComponent<PhysicsCMP>(entityKey);
    ASSERT(physicComp);

    PhysicsState *p = &physicComp->physics;

    u32 w = 72;
    u32 h = 28;
    
    u32 label_x = current_x + 10;
    u32 x = label_x + 40;

    if(_tgui_separator(window, "Physic Component", current_y, true, TGUI_ID)) {
        current_y += h + 10;

        _tgui_label(window, "p =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &p->pos.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->pos.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->pos.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "v =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &p->vel.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->vel.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->vel.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "a =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &p->acc.x, 0xff0000, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->acc.y, 0x00ff00, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &p->acc.z, 0x0000ff, x+2*w+20, current_y, w, TGUI_ID);
    }

    current_y += h;
}

void ComponentsWindow::UpdateCollisionComponent(Editor *editor, SlotmapKey entityKey) {
    CollisionCMP *col = editor->game->level.em.GetComponent<CollisionCMP>(entityKey);
    ASSERT(col);
    u32 w = 72;
    u32 h = 28;

    static char *options[] = {
        "Cylinder",
        "ConvexHull",
    };
    u32 optionsSize = sizeof(options)/sizeof(options[0]);

    u32 label_x = current_x + 10;
    u32 x = label_x + 40;

    if(_tgui_separator(window, "Collision Component", current_y, true, TGUI_ID)) {
        current_y += h + 10;
        _tgui_label(window, "c =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_dropdown_menu(window, x, current_y, options, optionsSize, (i32 *)&col->type, TGUI_ID); 
        current_y += 10;
    }
    current_y += h;
}

void ComponentsWindow::UpdateGraphicComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Graphic Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateInputComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Input Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdatePlayerAnimationComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Player Animation Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateMovingPlatformComponent(Editor *editor, SlotmapKey entityKey) {
    MovingPlatformCMP *movingComp = editor->game->level.em.GetComponent<MovingPlatformCMP>(entityKey);
    ASSERT(movingComp);
    u32 w = 72;
    u32 h = 28;

    Vec3 *a = &movingComp->a;
    Vec3 *b = &movingComp->b;

    u32 label_x = current_x + 10;
    u32 x = label_x + 40;

    if(_tgui_separator(window, "Moving Platform Component", current_y, true, TGUI_ID)) {

        current_y += h + 10;

        _tgui_label(window, "a =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &a->x, 0xffff00, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &a->y, 0x00ffff, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &a->z, 0xff00ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "b =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &b->x, 0xffff00, x, current_y, w, TGUI_ID);
        _tgui_float_input(window, &b->y, 0x00ffff, x+1*w+10, current_y, w, TGUI_ID);
        _tgui_float_input(window, &b->z, 0xff00ff, x+2*w+20, current_y, w, TGUI_ID);

        current_y += h;

        _tgui_label(window, "speed =", 0x222222, label_x, current_y, TGUI_ID);
        _tgui_float_input(window, &movingComp->speed, 0x00ff00, x + 40, current_y, w, TGUI_ID);
    }
    current_y += h;
}

void ComponentsWindow::UpdateAIComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "AI Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateTriggerComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Trigger Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateGemComponent(Editor *editor, SlotmapKey entityKey) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Gem Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}
