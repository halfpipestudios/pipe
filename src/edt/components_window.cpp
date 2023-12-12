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

void ComponentsWindow::Update(Editor *editor, f32 dt) {
    
    current_x = 10;
    current_y = 2;

    Entity_ *selectedEntity = editor->selectedEntity;

    if(selectedEntity) {
        
        auto *elements = selectedEntity->componentsPtrs.elements;
        u32 elementsCount = selectedEntity->componentsPtrs.capacity;

        for(u32 i = 0; i < elementsCount; ++i) {
            auto *element = &elements[i];
            if(element->id == 0) continue;

            CMPBase *component = element->value;

            if(component->id == TransformCMP::GetID()) {
                UpdateTransformComponent(selectedEntity);
            } else if(component->id == PhysicsCMP::GetID()) {
                UpdatePhysicsComponent(selectedEntity);
            } else if(component->id == CollisionCMP::GetID()) {
                UpdateCollisionComponent(selectedEntity);
            } else if(component->id == GraphicsCMP::GetID()) {
                UpdateGraphicComponent(selectedEntity);
            } else if(component->id == InputCMP::GetID()) {
                UpdateInputComponent(selectedEntity);
            } else if(component->id == AnimationCMP::GetID()) {
                UpdatePlayerAnimationComponent(selectedEntity);
            } else if(component->id == MovingPlatformCMP::GetID()) {
                UpdateMovingPlatformComponent(selectedEntity);
            } else if(component->id == AiCMP::GetID()) {
                UpdateAIComponent(selectedEntity);
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


void ComponentsWindow::UpdateTransformComponent(Entity_ *entity) {
    
    TransformCMP *transform = entity->GetComponent<TransformCMP>();
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

void ComponentsWindow::UpdatePhysicsComponent(Entity_ *entity) {
    PhysicsCMP *physicComp = entity->GetComponent<PhysicsCMP>();
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

void ComponentsWindow::UpdateCollisionComponent(Entity_ *entity) {
    CollisionCMP *col = entity->GetComponent<CollisionCMP>();
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

void ComponentsWindow::UpdateGraphicComponent(Entity_ *entity) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Graphic Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateInputComponent(Entity_ *entity) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Input Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdatePlayerAnimationComponent(Entity_ *entity) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Player Animation Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateMovingPlatformComponent(Entity_ *entity) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "Moving Platform Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}

void ComponentsWindow::UpdateAIComponent(Entity_ *entity) {
    u32 w = 72;
    u32 h = 28;
    if(_tgui_separator(window, "AI Component", current_y, false, TGUI_ID)) {
    }
    current_y += h;
}
