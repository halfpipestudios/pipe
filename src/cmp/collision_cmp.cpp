#include "collision_cmp.h"

void CollisionCMP::Initialize(Cylinder cylinder_) {
    type = COLLIDER_CYLINDER_;
    cylinder = cylinder_;
    active = true;
}

void CollisionCMP::Initialize(ConvexHull convexHull, MapImporter::Entity entity) {
    type = COLLIDER_CONVEXHULL_;
    poly3D.convexHull = convexHull;
    poly3D.entity = entity;
}
