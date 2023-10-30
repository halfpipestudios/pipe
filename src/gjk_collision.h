#ifndef _GJK_COLLISION_H_
#define _GJK_COLLISION_H_

#include "math.h"
#include "map_importer.h"

#include <vector>

struct ConvexHull {
    Vec3 *points;
    i32 count;

    Vec3 FindFurthestPoint(Vec3 dir);
};

struct Cylinder {
    Vec3 c;
    Vec3 u;
    f32 radii;
    f32 n;

    Vec3 FindFurthestPoint(Vec3 dir);
};


struct Point {
    Vec3 a, b;
    Vec3 p;
};

struct Simplex {
    union {
        Point points[4];
        struct {
            Point a, b, c, d;
        };

    };
    i32 count;

    void PushFront(Point point);
    void PushBack(Point point);
};

struct CollisionData {
    Vec3 normal;
    f32 penetration;
    bool hasCollision;
};


struct GJK {
    CollisionData Intersect(ConvexHull *a, ConvexHull *b);
    CollisionData Intersect(ConvexHull *a, Cylinder *b);

private: 
    Point Support(ConvexHull *a, Cylinder *b, Vec3 dir);
    Point Support(ConvexHull *a, ConvexHull *b, Vec3 dir);
    bool DoSimplex(Simplex &simplex, Vec3 &dir);
    bool Line(Simplex &simplex, Vec3 &dir);
    bool Triangle(Simplex &simplex, Vec3 &dir);
    bool Tetrahedron(Simplex &simplex, Vec3 &dir);
    bool SameDirection(Vec3 &dir, Vec3 &ao);


    CollisionData EPA(Simplex &simplex, ConvexHull *a, ConvexHull *b);
    CollisionData EPA(Simplex &simplex, ConvexHull *a, Cylinder *b);

    Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c);
    Vec3 ProjectPointOntoPlane(Plane &plane, Vec3 &point);
    void Barycentric(Vec3 &a, Vec3 &b, Vec3 &c, Vec3 &p, f32 &u, f32 &v, f32 &w);
};

#endif // _GJK_COLLISION_H_
