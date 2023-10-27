#ifndef _GJK_COLLISION_H_
#define _GJK_COLLISION_H_

#include "math.h"

struct ConvexHull {
    Vec3 *points;
    i32 count;

    Vec3 FindFurthestPoint(Vec3 dir);
};

struct Simplex {
    Vec3 points[4];
    i32 count;

    void PushFront(Vec3 point);
    void PushBack(Vec3 point);
};

struct GJK {
    bool Intersect(ConvexHull *a, ConvexHull *b);

private: 

    Vec3 Support(ConvexHull *a, ConvexHull *b, Vec3 dir);
    bool DoSimplex(Simplex &simplex, Vec3 &dir);
    bool Line(Simplex &simplex, Vec3 &dir);
    bool Triangle(Simplex &simplex, Vec3 &dir);
    bool Tetrahedron(Simplex &simplex, Vec3 &dir);
    bool SameDirection(Vec3 &dir, Vec3 &ao);
};



#endif // _GJK_COLLISION_H_
