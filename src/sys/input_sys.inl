#include "input_sys.h"

#include "../cmp/physics_cmp.h"

template <typename EM>
void InputSys<EM>::Update(EM& em, Camera *camera, f32 dt) {
    
    Input *input = PlatformManager::Get()->GetInput();
    auto& inputs = em.GetComponents<InputCMP>();

    Vec3 worldFront = camera->GetWorldFront();
    Vec3 right = camera->right;

    for(i32 i = 0; i < inputs.size; ++i) {
        InputCMP *inp = &inputs[i];
        SlotmapKey entityKey = inp->entityKey;
        Entity_ *entity = em.GetEntity(entityKey);
        PhysicsCMP *physicsComp = em.GetComponent<PhysicsCMP>(entityKey);
        if(physicsComp == nullptr) continue;

        if(input->KeyIsPress(KEY_W)) {
            physicsComp->physics.acc += worldFront;
        }
        if(input->KeyIsPress(KEY_S)) {
            physicsComp->physics.acc -= worldFront;
        }
        if(input->KeyIsPress(KEY_A)) {
            physicsComp->physics.acc -= right;
        }
        if(input->KeyIsPress(KEY_D)) {
            physicsComp->physics.acc += right;
        }

        if(entity->HaveFlag(ENTITY_GROUNDED) && (input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
            physicsComp->physics.vel += Vec3(0, 15, 0);
        }
        
        f32 acc = 40.0f;
        f32 drag = 1.0f;

        if(!entity->HaveFlag(ENTITY_GROUNDED)) {
            drag = 0.1f;
        }

        physicsComp->physics.acc += worldFront * input->state[0].leftStickY;
        physicsComp->physics.acc += right      * input->state[0].leftStickX;
        physicsComp->physics.acc.Normalize();
        physicsComp->physics.acc *= acc * drag;

        // TODO: quick fix for the view direction bug
        Vec3 dir = { physicsComp->viewDir.x, 0.0f, physicsComp->viewDir.z };
        dir.Normalize();
        f32 orientation = atan2f(dir.z, dir.x);
        physicsComp->physics.orientation = orientation;

    }

}
