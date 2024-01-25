#include "levitation_spell_sys.h"
#include "platform_manager.h"
#include "graphics_manager.h"
#include "globals.h"


template <typename EM>
void LevitationSpellSys<EM>::Update(EM& em, Camera *camera, f32 gameTime, f32 dt) {


    auto& movableBoxComponents = em.GetComponents<MovableBoxCMP>();
    auto& playerComponents = em.GetComponents<PlayerCMP>();

    // TODO: select the object that is closer to the camera front direction




    PlayerCMP *playerCmp = &playerComponents[0];

    if(PlatformManager::Get()->GetInput()->MouseJustPress(MOUSE_BUTTON_L) ||
       PlatformManager::Get()->GetInput()->JoystickJustPress(JOYSTICK_RIGHT_TRIGGER)) {

        f32 maxDot = 0;
        for(i32 i = 0; i < movableBoxComponents.size; i++) {
            MovableBoxCMP *mov = &movableBoxComponents[i];
            PhysicsCMP *phy = em.GetComponent<PhysicsCMP>(mov->entityKey);

            Vec3 relP = phy->physics.pos - camera->pos;
            Vec3 front = camera->front;

            f32 dot = relP.Dot(front);
            if(dot > maxDot) {
                selectedBox = mov;
                maxDot = dot;
            }
        }

        GraphicsManager::Get()->ResetParticleSystem(playerCmp->particleSys);
    
    }

    if(PlatformManager::Get()->GetInput()->MouseJustUp(MOUSE_BUTTON_L) ||
       PlatformManager::Get()->GetInput()->JoystickJustUp(JOYSTICK_RIGHT_TRIGGER)) {    
        selectedBox = nullptr;
    }

    // if we reach this point it means that we have selected a box to move
    if(selectedBox) {

        Entity_ *entity = em.GetEntity(selectedBox->entityKey);
        PhysicsCMP *phy = em.GetComponent<PhysicsCMP>(selectedBox->entityKey);
        TransformCMP *transform = em.GetComponent<TransformCMP>(selectedBox->entityKey);
        CollisionCMP *collider = em.GetComponent<CollisionCMP>(selectedBox->entityKey);

        PhysicsCMP *playPhy = em.GetComponent<PhysicsCMP>(playerCmp->entityKey);
        Vec3 pos = playPhy->physics.pos;
        
        // TODO: test to see if its better, moving the object with the camera directon
        // or the player direction
        Vec3 worldFront = camera->GetWorldFront();

        
        Vec3 newP = Vec3(pos.x, pos.y + 5, pos.z) + worldFront * 4.0f;
        phy->physics.acc = (newP - phy->physics.pos)*4.0f;
        phy->physics.acc.y *= 2.5f;

        GraphicsManager::Get()->UpdateParticleSystem(playerCmp->particleSys, pos, phy->physics.pos, camera->pos, gameTime, dt);
    }

}

template <typename EM>
void LevitationSpellSys<EM>::Render(EM& em) {
    if(PlatformManager::Get()->GetInput()->MouseIsPress(MOUSE_BUTTON_L) ||
       PlatformManager::Get()->GetInput()->JoystickIsPress(JOYSTICK_RIGHT_TRIGGER)) {
        auto& playerComponents = em.GetComponents<PlayerCMP>();
        PlayerCMP *playerCmp = &playerComponents[0];
        GraphicsManager::Get()->RenderParticleSystem(playerCmp->particleSys);
    }

}
