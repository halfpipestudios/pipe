#include "orc_sys.h"

template<typename EM>
void OrcSys<EM>::Update(EM& em, f32 dt) {
    auto& enemies = em.GetComponents<EnemyCMP>();
    for(i32 i = 0; i < enemies.size; ++i) {
        EnemyCMP *enemy = &enemies[i];
        PhysicsCMP *physicsComp = em.GetComponent<PhysicsCMP>(enemy->entityKey);
        enemy->sound.Update(physicsComp->physics.pos, physicsComp->physics.vel);
    }
}
