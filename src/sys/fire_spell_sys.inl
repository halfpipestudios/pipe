#include "fire_spell_sys.h"
#include "cmp/physics_cmp.h"
#include "graphics_manager.h"
#include "platform_manager.h"



static bool TestRayCircle(Vec3 p, Vec3 d, Vec3 cc, f32 cr, f32 &tOut) {
    Vec3 m = p - cc;
    f32 b = m.Dot(d);
    f32 c = m.Dot(m) - cr * cr;

    if(c > 0.0f && b > 0.0f) {
        tOut = FLT_MAX;
        return false;
    }

    f32 discr = b*b - c;

    if(discr < 0.0f) {
        tOut = FLT_MAX;
        return false;
    }
    
    f32 t = -b - sqrtf(discr);

    if(t < 0.0f) t = 0.0f;

    tOut = t;

    return true;
}

static bool CollisionTimeCircleCirlce(Vec3 c0, f32 r0, Vec3 v0,
                                      Vec3 c1, f32 r1, Vec3 v1) {            
    f32 radius = r0 + r1;
    Vec3 relV = v0 - v1;

    f32 distanSq = (c1 - c0).LenSq();
    f32 vlenSq = relV.LenSq();
    if (vlenSq < FLT_EPSILON && distanSq > radius * radius)
    {
        return false;
    }
    else if (vlenSq < FLT_EPSILON  && distanSq <= radius * radius)
    {
        return true;
    }
    else 
    {
        float vlen = sqrtf(vlenSq);
        relV = relV / vlen;
        f32 t = FLT_MAX;
        bool result = TestRayCircle(c0, relV, c1, radius, t);
        if(result) {

            t /= vlen;
            if(t <= 0.0f) {
                return true;
            }
        }
        return false;
    }

}


template <typename EM>
void FireSpellSys<EM>::Update(EM& em, Level *level, Vec3 cameraPos, f32 gameTime, f32 dt) {

    auto& fireSpellComponents = em.GetComponents<FireSpellCMP>();

    auto& enemyComponents = em.GetComponents<EnemyCMP>();

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
                
                fireball->sound.Play(false);
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


                for(i32 k = 0; k < enemyComponents.size; k++) {
                    EnemyCMP *ene = &enemyComponents[k];
                    PhysicsCMP *enemyPhy = em.GetComponent<PhysicsCMP>(ene->entityKey);

                    Vec3 ap = fireball->pos;
                    Vec3 av = fireball->vel;
                    f32  ar = 0.3f; 

                    Vec3 bp = enemyPhy->physics.pos;
                    Vec3 bv = enemyPhy->physics.vel;
                    f32  br = 0.3f; 

                    bp.y += 0.375f;
                    
                    if(CollisionTimeCircleCirlce(ap, ar, av, bp, br, bv)) {
                        printf("Hit\n");
                        level->DeleteEntity(ene->entityKey);
                        fireball->active = false;
                        break;
                    }

                }

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
