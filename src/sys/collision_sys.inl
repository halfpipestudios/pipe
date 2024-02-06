#include "collision_sys.h"
#include <float.h>

template <typename EM>
void CollisionSys<EM>::ProcessCylinderMap(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Map *map) {

    Entity_ *entity = em.GetEntity(collider->entityKey);

    Segment playerSegment;
    playerSegment.a =  phy->lastPhysics.pos;
    playerSegment.b = phy->physics.pos;

    f32 tMin = FLT_MAX; 
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(mapEntity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }
    if(tMin >= 0.0f && tMin <= 1.0f) {
        phy->physics.pos = phy->lastPhysics.pos + (phy->physics.pos - phy->lastPhysics.pos) * (tMin*0.8f);
        collider->cylinder.c = phy->physics.pos;
    }

    GJK gjk;
    for(u32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &collider->cylinder);
        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            phy->physics.pos += normal * penetration; 
            phy->physics.vel -= phy->physics.vel.Dot(normal)*normal;
            collider->cylinder.c = phy->physics.pos;
        }
    }

    Cylinder groundCylinder;
    groundCylinder.c = collider->cylinder.c - Vec3(0, collider->cylinder.n, 0);
    groundCylinder.u = Vec3(0, 1, 0);
    groundCylinder.radii = 0.29f;
    groundCylinder.n = 0.1f;
    bool colliding = false;
    for(u32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.IntersectFast(hull, &groundCylinder);
        if(collisionData.hasCollision) {
            colliding = true;
            break;
        }
    }
    if(colliding) {
        entity->AddFlag(ENTITY_GROUNDED);
    }

#if 0
    Segment groundSegment; 
    Vec3 lastVelXZ = Vec3(phy->physics.vel.x, 0.0f, phy->physics.vel.z);
    if(lastVelXZ.LenSq() > 0.0f) {
        phy->velXZ = lastVelXZ.Normalized();
    }

    groundSegment.a = collider->cylinder.c - (phy->velXZ * (collider->cylinder.radii + 0.05f));
    groundSegment.b = groundSegment.a + Vec3(0, -(collider->cylinder.n + 0.01f), 0);

    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

    groundSegment.a = collider->cylinder.c;
    groundSegment.b = groundSegment.a + Vec3(0, -(collider->cylinder.n + 0.01f), 0);

    GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, 0xFFFF0000);
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }
#endif

}

template <typename EM>
void CollisionSys<EM>::ProcessCylinderColliders(EM& em, CollisionCMP *collider, PhysicsCMP *phy, Array<CollisionCMP>* colliders) {

    GJK gjk;
    Entity_ *entity = em.GetEntity(collider->entityKey);

    for(i32 i = 0; i < colliders->size; ++i) {
        CollisionCMP *otherCollider = &(*colliders)[i];

        // not collide with itself ...
        if(collider == otherCollider) continue;
        if(otherCollider->active == false) continue;
 
        CollisionData collisionData = {};
        switch(otherCollider->type) {
            case COLLIDER_CYLINDER_: {
                collisionData = gjk.Intersect(&otherCollider->cylinder, &collider->cylinder);
            } break;
            case COLLIDER_CONVEXHULL_: {
                collisionData = gjk.Intersect(&otherCollider->poly3D.convexHull, &collider->cylinder);
            } break;
        }

        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            phy->physics.pos += normal * penetration; 
            phy->physics.vel -= phy->physics.vel.Dot(normal)*normal;
            collider->cylinder.c = phy->physics.pos;
        }

        Cylinder groundCylinder;
        groundCylinder.c = collider->cylinder.c - Vec3(0, collider->cylinder.n, 0);
        groundCylinder.u = Vec3(0, 1, 0);
        groundCylinder.radii = 0.29f;
        groundCylinder.n = 0.1f;

        switch(otherCollider->type) {
            case COLLIDER_CYLINDER_: {
                collisionData = gjk.IntersectFast(&otherCollider->cylinder, &groundCylinder);
            } break;
            case COLLIDER_CONVEXHULL_: {
                collisionData = gjk.IntersectFast(&otherCollider->poly3D.convexHull, &groundCylinder);
            } break;
        }

        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_GROUNDED);
            // TODO: move this to other place
            // move the player realtive to the platofrm position
            MovingPlatformCMP *movPlatComp = em.GetComponent<MovingPlatformCMP>(otherCollider->entityKey);
            if(movPlatComp != nullptr) {
                phy->physics.pos  = phy->physics.pos + movPlatComp->movement;
                collider->cylinder.c = phy->physics.pos;
            }
        }

#if 0

        Vec3 lastVelXZ = Vec3(phy->physics.vel.x, 0.0f, phy->physics.vel.z);
        if(lastVelXZ.LenSq() > 0.0f) {
            phy->velXZ = lastVelXZ.Normalized();
        }

        Segment groundSegment;
        groundSegment.a = collider->cylinder.c - (phy->velXZ * (collider->cylinder.radii + 0.05f));
        groundSegment.b = groundSegment.a + Vec3(0, -(collider->cylinder.n + 0.1f), 0);

        Segment centerGroundSegment;
        centerGroundSegment.a = collider->cylinder.c;
        centerGroundSegment.b = centerGroundSegment.a + Vec3(0, -(collider->cylinder.n + 0.1f), 0);

        f32 t = -1.0f;
        if(groundSegment.HitCollider(otherCollider, &t) ||
           centerGroundSegment.HitCollider(otherCollider, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);

            // TODO: move this to other place
            // move the player realtive to the platofrm position
            MovingPlatformCMP *movPlatComp = em.GetComponent<MovingPlatformCMP>(otherCollider->entityKey);
            if(movPlatComp != nullptr) {
                phy->physics.pos  = phy->physics.pos + movPlatComp->movement;
                collider->cylinder.c = phy->physics.pos;
            }
        }
#endif
    }

}


template <typename EM>
void CollisionSys<EM>::ProcessConvexHullMap(EM& em, CollisionCMP *collider, PhysicsCMP *phy, TransformCMP *transform, Map *map) {

    Entity_ *entity = em.GetEntity(collider->entityKey);

    Segment playerSegment;
    playerSegment.a =  phy->lastPhysics.pos;
    playerSegment.b = phy->physics.pos;

    f32 tMin = FLT_MAX; 
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(mapEntity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }

    if(tMin >= 0.0f && tMin <= 1.0f) {
        phy->physics.pos = phy->lastPhysics.pos + (phy->physics.pos - phy->lastPhysics.pos) * (tMin*0.8f);
        transform->pos = phy->physics.pos;
        TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
        TransformEntity(&collider->poly3D.entity, transform->scale, phy->physics.pos);
    }

    GJK gjk;
    for(u32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &collider->poly3D.convexHull);
        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            phy->physics.pos += normal * penetration; 
            phy->physics.vel -= phy->physics.vel.Dot(normal)*normal;

            transform->pos = phy->physics.pos;
            TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
            TransformEntity(&collider->poly3D.entity, transform->scale, phy->physics.pos);
        }
    }
    
    // create four line on the vertex of the box going down to detect if is grounded
    Vec3 bottomCenter = phy->physics.pos - (Vec3(0, transform->scale.y, 0) * 0.5f);

    Segment lt;
    lt.a = bottomCenter + (Vec3(-transform->scale.x, 0, transform->scale.z) * 0.5f);
    lt.b = lt.a - Vec3(0, 0.1f, 0); 

    Segment rt;
    rt.a = bottomCenter + (Vec3(transform->scale.x, 0, transform->scale.z) * 0.5f);
    rt.b = rt.a - Vec3(0, 0.1f, 0); 

    Segment rb;
    rb.a = bottomCenter + (Vec3(transform->scale.x, 0, -transform->scale.z) * 0.5f);
    rb.b = rb.a - Vec3(0, 0.1f, 0); 

    Segment lb;
    lb.a = bottomCenter + (Vec3(-transform->scale.x, 0, -transform->scale.z) * 0.5f);
    lb.b = lb.a - Vec3(0, 0.1f, 0); 

    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(lt.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
        else if(rt.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
        else if(rb.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
        else if(lb.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

}

template<typename EM> 
void CollisionSys<EM>::ProcessConvexHullColliders(EM& em, CollisionCMP *collider, PhysicsCMP *phy, TransformCMP *transform, Array<CollisionCMP>* colliders) {

    GJK gjk;
    Entity_ *entity = em.GetEntity(collider->entityKey);

    for(i32 i = 0; i < colliders->size; ++i) {
        CollisionCMP *otherCollider = &(*colliders)[i];

        // not collide with itself ...
        if(collider == otherCollider) continue;
        if(otherCollider->active == false) continue;
        if(otherCollider->type == COLLIDER_CYLINDER_) continue;
 
        CollisionData collisionData = gjk.Intersect(&otherCollider->poly3D.convexHull, &collider->poly3D.convexHull);

        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            phy->physics.pos += normal * penetration; 
            phy->physics.vel -= phy->physics.vel.Dot(normal)*normal;

            transform->pos = phy->physics.pos;
            TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
            TransformEntity(&collider->poly3D.entity, transform->scale, phy->physics.pos);
        }

        // create four line on the vertex of the box going down to detect if is grounded
        Vec3 bottomCenter = phy->physics.pos - (Vec3(0, transform->scale.y, 0) * 0.5f);

        Segment lt;
        lt.a = bottomCenter + (Vec3(-transform->scale.x, 0, transform->scale.z) * 0.5f);
        lt.b = lt.a - Vec3(0, 0.1f, 0); 

        Segment rt;
        rt.a = bottomCenter + (Vec3(transform->scale.x, 0, transform->scale.z) * 0.5f);
        rt.b = rt.a - Vec3(0, 0.1f, 0); 

        Segment rb;
        rb.a = bottomCenter + (Vec3(transform->scale.x, 0, -transform->scale.z) * 0.5f);
        rb.b = rb.a - Vec3(0, 0.1f, 0); 

        Segment lb;
        lb.a = bottomCenter + (Vec3(-transform->scale.x, 0, -transform->scale.z) * 0.5f);
        lb.b = lb.a - Vec3(0, 0.1f, 0); 


        f32 t = -1.0f;
        if(lt.HitCollider(otherCollider, &t) ||
           rt.HitCollider(otherCollider, &t) ||
           rb.HitCollider(otherCollider, &t) ||
           lb.HitCollider(otherCollider, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);

            MovingPlatformCMP *movPlatComp = em.GetComponent<MovingPlatformCMP>(otherCollider->entityKey);
            if(movPlatComp != nullptr) {
                phy->physics.pos  = phy->physics.pos + movPlatComp->movement;

                transform->pos = phy->physics.pos;
                TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
                TransformEntity(&collider->poly3D.entity, transform->scale, phy->physics.pos);
            }
        }
    }

}

template <typename EM>
void CollisionSys<EM>::Update(EM& em, Map *map, f32 dt) {

    auto& colliders = em.GetComponents<CollisionCMP>();
    for(i32 i = 0; i < colliders.size; ++i) {

        CollisionCMP *collider = &colliders[i];
        if(collider->active == false) continue;

        SlotmapKey entityKey = collider->entityKey;
        Entity_ *entity = em.GetEntity(entityKey);
        PhysicsCMP *phy = em.GetComponent<PhysicsCMP>(entityKey);
        TransformCMP *transform = em.GetComponent<TransformCMP>(entityKey);
        if(phy == nullptr) continue;

        if(collider->type == COLLIDER_CYLINDER_) {
            collider->cylinder.c = phy->physics.pos;
            entity->RemoveFlag(ENTITY_GROUNDED);
            entity->RemoveFlag(ENTITY_COLLIDING); 
            ProcessCylinderMap(em, collider, phy, map);
            ProcessCylinderColliders(em, collider, phy, &colliders);
        }
        if(collider->type == COLLIDER_CONVEXHULL_) {
            transform->pos = phy->physics.pos;
            TransformCube(collider->poly3D.convexHull.points, transform->GetWorldMatrix());
            TransformEntity(&collider->poly3D.entity, transform->scale, phy->physics.pos);
            entity->RemoveFlag(ENTITY_GROUNDED);
            entity->RemoveFlag(ENTITY_COLLIDING); 
            ProcessConvexHullMap(em, collider, phy, transform, map);
            ProcessConvexHullColliders(em, collider, phy, transform, &colliders);
        }

    }

}
