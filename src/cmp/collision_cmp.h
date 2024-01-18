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
    
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "collision");
        Write(s, "type", (i32)type);
        switch(type) {
            case COLLIDER_CYLINDER_: {
                WriteBeginObject(s, "cylinder");
                Write(s, "c", cylinder.c);
                Write(s, "u", cylinder.u);
                Write(s, "radii", cylinder.radii);
                Write(s, "n", cylinder.n);
                WriteEndObject(s);
            } break;
            case COLLIDER_CONVEXHULL_: {
                // Convex Hull
                WriteBeginObject(s, "convex_hull");
                Write(s, "count", poly3D.convexHull.count);
                for(i32 i = 0; i < poly3D.convexHull.count; i++) {
                    Write(s, "v", poly3D.convexHull.points[i]);
                }
                WriteEndObject(s);

                // Map Importer Entity
                WriteBeginObject(s, "map_importer_entity");
                Write(s, "faces_count", poly3D.entity.facesCount);
                for(i32 i = 0; i < poly3D.entity.facesCount; i++) {
                    WriteBeginObject(s, "plane");
                    Write(s, "n", poly3D.entity.faces[i].plane.n);
                    Write(s, "d", poly3D.entity.faces[i].plane.d);
                    WriteEndObject(s);
                }
                WriteEndObject(s);
            } break;
        } 
        WriteEndObject(s);
    };


    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "collision");
        Read(t, "type", &(i32)type);

        switch(type) {
            case COLLIDER_CYLINDER_: {
                Cylinder cylinder_;
                ReadBeginObject(t, "cylinder");
                Read(t, "c", &cylinder_.c);
                Read(t, "u", &cylinder_.u);
                Read(t, "radii", &cylinder_.radii);
                Read(t, "n", &cylinder_.n);
                ReadEndObject(t);
                Initialize(cylinder_);
            } break;
            case COLLIDER_CONVEXHULL_: {
                ConvexHull convexHull_;
                ReadBeginObject(t, "convex_hull");
                Read(t, "count", &convexHull_.count);
                convexHull_.points = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(convexHull_.count * sizeof(Vec3), 8);
                for(i32 i = 0; i < convexHull_.count; i++) {
                    Read(t, "v", convexHull_.points + i);
                }
                ReadEndObject(t);
                MapImporter::Entity entity_ = {};
                ReadBeginObject(t, "map_importer_entity");
                Read(t, "faces_count", &entity_.facesCount);
                entity_.faces = (MapImporter::EntityFace *)MemoryManager::Get()->AllocStaticMemory(sizeof(MapImporter::EntityFace) * entity_.facesCount, 8);
                memset(entity_.faces, 0, sizeof(MapImporter::EntityFace) * entity_.facesCount);
                for(i32 i = 0; i < entity_.facesCount; i++) {
                    ReadBeginObject(t, "plane");
                    Read(t, "n", &entity_.faces[i].plane.n);
                    Read(t, "d", &entity_.faces[i].plane.d);
                    ReadEndObject(t);
                }
                ReadEndObject(t);
                Initialize(convexHull_, entity_);
            } break;
        }


        ReadEndObject(t);
    };


};

#endif // _COLLISION_CMP_H_
