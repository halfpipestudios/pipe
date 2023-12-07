#include "transform_sys.h"

#include "../cmp/physics_cmp.h"

template <typename EM>
void TransformSys<EM>::Update(EM& em) {
    
    auto& transforms = em.GetComponents<TransformCMP>();

    for(i32 i = 0; i < transforms.size; ++i) {
        TransformCMP *transform = &transforms[i];
        Entity_ *entity = transform->entity;
        PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();

        if(physicsComp == nullptr) continue;

        transform->pos = physicsComp->physics.pos;
        transform->rot.y = physicsComp->physics.orientation - (f32)PI*0.5f;

    }

}
