#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "common.h"
#include "math.h"

#include "map_importer.h"

struct CollisionCMP;
struct CollisionComponent;
struct Cylinder;

struct Segment {
    Vec3 a;
    Vec3 b;

    bool HitEntity(MapImporter::Entity *entity, f32 *tOut);
    bool HitCollider(CollisionComponent *collider, f32  *tOut);
    bool HitCollider(CollisionCMP *collider, f32  *tOut);

    bool HitCylinder(Cylinder *cylinder, f32 *tOut);
    bool HitConvexHull(ConvexHull *convexHull, f32 *tOut);
};

#endif
