#include "geometry.h"
#include "cmp/collision_cmp.h"

bool Segment::HitEntity(MapImporter::Entity *entity, f32 *tOut) {

    if(entity->faces <= 0) {
        *tOut = -1.0f;
        return false;
    }

    Vec3 d = b - a;
    // Set initial interval to being the whole segment. For a ray, tlast should be
    // sety to FLT_MAX. For a line tfirst should be set to - FLT_MAX
    f32 tFirst = 0.0f;
    f32 tLast = 1.0f;
    // intersect segment agains each plane
    for(i32 i = 0; i < entity->facesCount; i++)
    {
        Plane p = entity->faces[i].plane;

        f32 denom = p.n.Dot(d);
        f32 dot = p.n.Dot(a);
        f32 dist = dot - (p.d/32.0f);
        // test if segment runs parallel to tha plane
        if(denom == 0.0f)
        {
            // If so, return "no intersection" if segemnt lies outside the plane
            if(dist > 0.0f) return 0;
        }
        else
        {
            f32 t = -(dist / denom);
            if(denom < 0.0f)
            {
                // when entering halfspace, update tfirst if t is larger
                if(t > tFirst) tFirst = t;
            }
            else
            {
                // when exiting halfspace, update tLast if t is smaller
                if(t < tLast) tLast = t;
            }

            if(tFirst > tLast) return 0;
        }
    }
    *tOut = tFirst;
    return 1;
}

bool Segment::HitCylinder(Cylinder *cylinder, f32 *tOut) {
    *tOut = -1.0f;
    return false;
}

bool Segment::HitCollider(CollisionCMP *collider, f32  *tOut) {

    switch(collider->type) {
        case COLLIDER_CYLINDER_: {
            return HitCylinder(&collider->cylinder, tOut);
        } break;

        case COLLIDER_CONVEXHULL_: {
            return HitEntity(&collider->poly3D.entity, tOut);
        } break;
    }

    *tOut = -1.0f;
    return false;
}

Vec3 Ray::IntersectPlane(Vec3 p, Vec3 n) {

    f32 t = 0;
    if(ABS(d.Dot(n)) > 0.01f)  {
        t = (p.Dot(n) - o.Dot(n)) / d.Dot(n);
    }

    Vec3 result = o + d * t; 

    return result;
}
