#include "fire_spell_sys.h"
#include "cmp/physics_cmp.h"
#include "graphics_manager.h"
#include "platform_manager.h"

template <typename EM>
void FireSpellSys<EM>::Update(EM& em, Vec3 cameraPos, f32 gameTime, f32 dt) {

    auto& fireSpellComponents = em.GetComponents<FireSpellCMP>();

    for(i32 i = 0; i < fireSpellComponents.size; i++) {
        FireSpellCMP *fir = &fireSpellComponents[i];
        SlotmapKey entity = fir->entityKey;
        PhysicsCMP *phy = em.GetComponent<PhysicsCMP>(entity);

        if(phy == nullptr) continue;

        float dirX = cosf(phy->physics.orientation);
        float dirZ = sinf(phy->physics.orientation);
        Vec3 shootDir = Vec3(dirX, 0.0f, dirZ);

        fir->timer += dt;
        Input *input = PlatformManager::Get()->GetInput();
        if(input->KeyIsPress(KEY_K) ||
           input->JoystickIsPress(JOYSTICK_BUTTON_X)) {
            if(fir->timer >= fir->cooldown) {
                fir->timer = 0.0;

                Fireball *fireball = fir->fireballs + fir->nextToShoot;
                
                fireball->active = true;
                fireball->life = 0.0f;
                fireball->pos = phy->physics.pos;
                fireball->vel = shootDir * fir->speed;

                GraphicsManager::Get()->ResetParticleSystem(fireball->particleSys);
                fir->nextToShoot = (fir->nextToShoot + 1) % ARRAY_LENGTH(fir->fireballs);
            }
        }

        for(i32 j = 0; j < ARRAY_LENGTH(fir->fireballs); j++) {
            Fireball *fireball = fir->fireballs + j;
            fireball->renderTimer += dt;
            if(fireball->active) {
                
                fireball->life += dt;
                if(fireball->life >= fir->lifeTime) {
                    fireball->active = false;
                }

                fireball->pos = fireball->pos + fireball->vel * dt;

                Vec3 o = fireball->pos;
                Vec3 f = fireball->vel.Normalized();
                Vec3 r = Vec3(0, 1, 0).Cross(f);
                Vec3 u = f.Cross(r);
                
                Vec3 offset = Mat4::TransformVector(Mat4::RotateZ(fireball->renderTimer*fireball->spinDir*20.0f), Vec3(1, 0, 0)) * 0.15f;
                Vec3 localP = Vec3(0, 0, 0) + offset;

                Mat4 transformMatrix = Mat4::TransformFromBasis(o, r, u, f);
                Vec3 worldP = Mat4::TransformPoint(transformMatrix, localP);

                GraphicsManager::Get()->UpdateParticleSystem(fireball->particleSys,
                            worldP, cameraPos, gameTime, dt);
            }
        }


    }

}

template <typename EM>
void FireSpellSys<EM>::Render(EM& em) {

    auto& fireSpellComponents = em.GetComponents<FireSpellCMP>();

    for(i32 i = 0; i < fireSpellComponents.size; i++) {

        FireSpellCMP *fir = &fireSpellComponents[i];
        for(i32 j = 0; j < ARRAY_LENGTH(fir->fireballs); j++) {
            Fireball *fireball = fir->fireballs + j;
            if(fireball->active) {
                GraphicsManager::Get()->RenderParticleSystem(fireball->particleSys);                
            }
        }

    }
}
