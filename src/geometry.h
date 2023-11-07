#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "common.h"
#include "math.h"

#include "map_importer.h"

struct Segment {
    Vec3 a;
    Vec3 b;

    bool HitEntity(MapImporter::Entity *entity, f32 *tOut);
};

#endif
