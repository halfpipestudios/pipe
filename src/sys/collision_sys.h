#ifndef _COLLISION_SYS_H_
#define _COLLISION_SYS_H_

#include "..\cmp\collision_cmp.h"

template <typename EM>
struct CollisionSys {
    void Update(EM& em, Map *map, f32 dt);

private:
    void ProcessMap(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Map *map);
    void ProcessColliders(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Array<CollisionCMP>* colliders);
};

#endif // _COLLISION_SYS_H_
