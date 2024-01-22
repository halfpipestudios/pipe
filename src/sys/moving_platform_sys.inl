#include "moving_platform_sys.h"

#include "globals.h"

template <typename EM>
void MovingPlatformSys<EM>::Update(EM& em, f32 dt) {

    auto& platforms = em.GetComponents<MovingPlatformCMP>();

    for(i32 i = 0; i < platforms.size; ++i) {
        
        MovingPlatformCMP *platform = &platforms[i];
        SlotmapKey entityKey = platform->entityKey;

        TransformCMP *transform = em.GetComponent<TransformCMP>(entityKey);
        CollisionCMP *collider  = em.GetComponent<CollisionCMP>(entityKey);

        if(transform == nullptr || collider == nullptr) continue;


        Vec3 lastPos = transform->pos;
        Vec3 ab = platform->b - platform->a;
        f32 len = ab.Len();
        
        if(len > 0.001f) {
            f32 t = (sinf((platform->dtElapsed / len) * platform->speed) + 1.0f) * 0.5f;
            transform->pos = platform->a + (platform->b - platform->a) * t;
        }

        platform->movement = transform->pos - lastPos;
        TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
        TransformEntity(&collider->poly3D.entity, transform->scale, transform->pos);

        platform->dtElapsed += dt; 
        
    }

}
