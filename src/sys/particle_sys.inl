#include "particle_sys.h"
#include "../graphics_manager.h"


#include "../cmp/physics_cmp.h"

template <typename EM>
void ParticleSys<EM>::Update(EM& em, Vec3 cameraPos, f32 gameTime, f32 dt) {

    auto& particleComponents = em.GetComponents<ParticleCMP>();

    for(i32 i = 0; i < particleComponents.size; ++i) {

        ParticleCMP *par = &particleComponents[i];
        SlotmapKey e = par->entityKey;
        TransformCMP *transform = em.template GetComponent<TransformCMP>(e);

        GraphicsManager::Get()->UpdateParticleSystem(par->particleSys,
                transform->pos, cameraPos, gameTime, dt);


    }


}

template <typename EM>
void ParticleSys<EM>::Render(EM& em) {

    auto& particleComponents = em.GetComponents<ParticleCMP>();

    for(i32 i = 0; i < particleComponents.size; ++i) {

        ParticleCMP *par = &particleComponents[i];
        GraphicsManager::Get()->RenderParticleSystem(par->particleSys);

    }


}


