#ifndef _COLLISION_SYS_H_
#define _COLLISION_SYS_H_

#include "..\cmp\collision_cmp.h"

struct TransformCMP;

template <typename EM>
struct CollisionSys {
    void Update(EM& em, Map *map, f32 dt);

private:
    void ProcessCylinderMap(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Map *map);
    void ProcessCylinderColliders(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Array<CollisionCMP>* colliders);


    void ProcessConvexHullMap(EM& em, CollisionCMP *collider, PhysicsCMP *phy, TransformCMP *transform, Map *map);
    void ProcessConvexHullColliders(EM& em, CollisionCMP *collider, PhysicsCMP *phy, TransformCMP *transform, Array<CollisionCMP>* colliders);
};

#endif // _COLLISION_SYS_H_
