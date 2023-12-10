#include "collision_sys.h"
#include <float.h>

template <typename EM>
void CollisionSys<EM>::ProcessMap(CollisionCMP *collider, PhysicsCMP *phy, Map *map) {

    Entity_ *entity = collider->entity;

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
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

}

template <typename EM>
void CollisionSys<EM>::ProcessColliders(CollisionCMP *collider, PhysicsCMP *phy, Array<CollisionCMP>* colliders) {

    GJK gjk;
    Entity_ *entity = collider->entity;

    for(i32 i = 0; i < colliders->size; ++i) {
        CollisionCMP *otherCollider = &(*colliders)[i];

        // not collide with itself ...
        if(collider == otherCollider) continue;


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
 
        CollisionData collisionData = {};
        switch(otherCollider->type) {
            case COLLIDER_CYLINDER_: {
                collisionData = gjk.Intersect(&otherCollider->cylinder, &collider->cylinder);
            } break;
            case COLLIDER_CONVEXHULL_: {
                collisionData = gjk.Intersect(&otherCollider->poly3D.convexHull, &collider->cylinder);
            }
        }

        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            phy->physics.pos += normal * penetration; 
            phy->physics.vel -= phy->physics.vel.Dot(normal)*normal;
            collider->cylinder.c = phy->physics.pos;
        }

        f32 t = -1.0f;
        if(groundSegment.HitCollider(otherCollider, &t) ||
           centerGroundSegment.HitCollider(otherCollider, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);

            // TODO: move this to other place
            // move the player realtive to the platofrm position
            MovingPlatformCMP *movPlatComp = otherCollider->entity->GetComponent<MovingPlatformCMP>();
            if(movPlatComp != nullptr) {
                phy->physics.pos  = phy->physics.pos + movPlatComp->movement;
                collider->cylinder.c = phy->physics.pos;
            }
        }
    }

}

template <typename EM>
void CollisionSys<EM>::Update(EM& em, Map *map, f32 dt) {

    auto& colliders = em.GetComponents<CollisionCMP>();
    for(i32 i = 0; i < colliders.size; ++i) {

        CollisionCMP *collider = &colliders[i];
        Entity_ *entity = collider->entity;
        PhysicsCMP *phy = entity->GetComponent<PhysicsCMP>();
        if(phy == nullptr) continue;


        // TODO: more colliders
        if(collider->type == COLLIDER_CYLINDER_) {
            collider->cylinder.c = phy->physics.pos;
            entity->RemoveFlag(ENTITY_GROUNDED);
            entity->RemoveFlag(ENTITY_COLLIDING); 
            ProcessMap(collider, phy, map);
            ProcessColliders(collider, phy, &colliders);
        }


    }

}
