#include "input_sys.h"

#include "../cmp/physics_cmp.h"

template <typename EM>
void InputSys<EM>::Update(EM& em, f32 dt) {
    
    auto& inputs = em.GetComponents<InputCMP>();

    for(i32 i = 0; i < inputs.size; ++i) {
        InputCMP *inp = &inputs[i];
        Entity_ *entity = inp->entity;
        PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
        if(physicsComp == nullptr) continue;

        Vec3 worldFront = inp->camera->GetWorldFront();
        Vec3 right = inp->camera->right;
        if(inp->input->KeyIsPress(KEY_W)) {
            physicsComp->physics.acc += worldFront;
        }
        if(inp->input->KeyIsPress(KEY_S)) {
            physicsComp->physics.acc -= worldFront;
        }
        if(inp->input->KeyIsPress(KEY_A)) {
            physicsComp->physics.acc -= right;
        }
        if(inp->input->KeyIsPress(KEY_D)) {
            physicsComp->physics.acc += right;
        }

        if(entity->HaveFlag(ENTITY_GROUNDED) && (inp->input->KeyJustPress(KEY_SPACE) || inp->input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
            physicsComp->physics.vel += Vec3(0, 15, 0);
        }
        
        f32 acc = 40.0f;
        f32 drag = 1.0f;

        if(!entity->HaveFlag(ENTITY_GROUNDED)) {
            drag = 0.1f;
        }

        physicsComp->physics.acc += worldFront * inp->input->state[0].leftStickY;
        physicsComp->physics.acc += right      * inp->input->state[0].leftStickX;
        physicsComp->physics.acc *= acc * drag;


        // TODO: the player orientatin should not depend on the velocity direction,
        // the velocity direction should depend on the angular velocity. 
        // TODO: add and angular velocity to the player
        Vec3 dir = { physicsComp->physics.vel.x, 0.0f, physicsComp->physics.vel.z };
        dir.Normalize();
        f32 orientation = atan2f(dir.z, dir.x);
        physicsComp->physics.orientation = orientation;

    }

}
