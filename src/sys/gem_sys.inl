#include "gem_sys.h"


template <typename EM>
void GemSys<EM>::GrabGems(Level *level, GemCMP *gem, 
                          TransformCMP *transform, TriggerCMP *trigger,
                          TransformCMP *whoTransform, CollisionCMP *whoCollider,
                          f32 dt) {
    GJK gjk;
    CollisionData collisionData = {};
    switch(whoCollider->type) {
        case COLLIDER_CYLINDER_: {
            collisionData = gjk.IntersectFast(&whoCollider->cylinder, &trigger->collider);
        } break;
        case COLLIDER_CONVEXHULL_: {
            collisionData = gjk.IntersectFast(&whoCollider->poly3D.convexHull, &trigger->collider);
        } break;
    }

    if(collisionData.hasCollision) {
        gem->sound.Play(false);
        level->DeleteEntity(gem->entityKey); 
        printf("gem catched, value: %d\n", gem->value);
    }

}

template <typename EM>
void GemSys<EM>::UpdateGem(GemCMP *gem, TransformCMP *transform, TriggerCMP *trigger, f32 dt) {
    gem->timer += dt;
    transform->renderOffset.y = sinf((gem->timer*8) + gem->timerOffset) * 0.08f;
    transform->rot.y = gem->timer; 
}


template <typename EM>
void GemSys<EM>::Update(EM& em, Level *level, f32 dt) {

    auto& gems = em.GetComponents<GemCMP>();

    for(i32 i = 0; i < gems.size; ++i) {
        GemCMP *gem = &gems[i];
        TriggerCMP *trigger = em.GetComponent<TriggerCMP>(gem->entityKey);
        TransformCMP *transform = em.GetComponent<TransformCMP>(gem->entityKey);

        if(trigger == nullptr || transform == nullptr) continue;

        CollisionCMP *whoCollider = em.GetComponent<CollisionCMP>(trigger->whoTriggerThis);
        TransformCMP *whoTransform = em.GetComponent<TransformCMP>(trigger->whoTriggerThis);

        if(whoCollider == nullptr ||whoTransform == nullptr ) continue;

        UpdateGem(gem, transform, trigger, dt);

        Vec3 toGem = whoTransform->pos - transform->pos;
        if(toGem.Len() <= 1.5f) {
            transform->pos += toGem.Normalized() * 6 * dt;
        }
        else {
            continue;
        }
        GrabGems(level, gem, transform, trigger, whoTransform, whoCollider, dt);
    }


}
