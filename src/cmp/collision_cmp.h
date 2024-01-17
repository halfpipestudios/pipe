#ifndef _COLLISION_CMP_H_
#define _COLLISION_CMP_H_

#include "../gjk_collision.h"
#include "../map_importer.h"
#include "base_cmp.h"

enum ColliderType_ {
    COLLIDER_CYLINDER_,
    COLLIDER_CONVEXHULL_
};

struct CollisionCMP : CMP<CollisionCMP> {

    // TODO(IMPORTANT): change this component 
    // this is ugly ...

    ColliderType_ type;
    union {
    
    Cylinder cylinder;
    struct Poly3D {
        ConvexHull convexHull;
        // TODO: not use a MapImporter::Entity here ...
        MapImporter::Entity entity;
    } poly3D;

    };

    CollisionCMP() {}
    void Initialize(Cylinder cylinder);
    void Initialize(ConvexHull convexHull, MapImporter::Entity entity);
    
    char *TypeToStr() {
        switch(type) {
            case COLLIDER_CYLINDER_: return "cylinder";
            case COLLIDER_CONVEXHULL_: return "convex_hull";
        }
        ASSERT(!"invalid code path");
        return nullptr;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "collision");
        Write(s, "type", TypeToStr());
        WriteEndObject(s);
    };

};

#endif // _COLLISION_CMP_H_
