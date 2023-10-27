#include "gjk_collision.h"
#include <float.h>


Vec3 ConvexHull::FindFurthestPoint(Vec3 dir) {
    Vec3 maxPoint;
    f32 maxDist = -FLT_MAX;
    for(i32 i = 0; i < count; ++i) {
        Vec3 point = points[i];
        f32 dist = point.Dot(dir);
        if(dist > maxDist) {
            maxDist = dist;
            maxPoint = point;
        }
    }
    return maxPoint;
}


void Simplex::PushFront(Vec3 point) {
    points[3] = points[2];
    points[2] = points[1];
    points[1] = points[0];
    points[0] = point;
    count = MIN(count + 1, 4);
}

void Simplex::PushBack(Vec3 point) {
    ASSERT(count < 4);
    points[count++] = point;
}


Vec3 GJK::Support(ConvexHull *a, ConvexHull *b, Vec3 dir) {
    return a->FindFurthestPoint(dir) - b->FindFurthestPoint(dir * -1.0f); 
}


bool GJK::Intersect(ConvexHull *a, ConvexHull *b) {

    Vec3 support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support * -1.0f;

    while(true) {
        support = Support(a, b, dir);

        if(support.Dot(dir) <= 0) {
            return false;
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            return true;
        }

    }

}

bool GJK::DoSimplex(Simplex &simplex, Vec3 &dir) {
    switch(simplex.count) {
        case 2: return Line(simplex, dir);
        case 3: return Triangle(simplex, dir);
        case 4: return Tetrahedron(simplex, dir);
    } 
    return false;
}

bool GJK::Line(Simplex &simplex, Vec3 &dir) {

    Vec3 a = simplex.points[0];
    Vec3 b = simplex.points[1];

    Vec3 ab = b - a;
    Vec3 ao = a * -1.0f;

    if(SameDirection(ab, ao)) {
        dir = ab.Cross(ao).Cross(ab);
    }
    else {
        simplex = {};
        simplex.PushBack(a);
        dir = ao;
    }

    return false;
}

bool GJK::Triangle(Simplex &simplex, Vec3 &dir) {

    Vec3 a = simplex.points[0];
    Vec3 b = simplex.points[1];
    Vec3 c = simplex.points[2];

    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 ao = a * -1.0f;

    Vec3 abc = ab.Cross(ac);

    if(SameDirection(abc.Cross(ac), ao)) {
        if(SameDirection(ac, ao)) {
            simplex = {};
            simplex.PushBack(a);
            simplex.PushBack(c);
            dir = ac.Cross(ao).Cross(ac);
        }
        else {
            simplex = {};
            simplex.PushBack(a);
            simplex.PushBack(b);
            return Line(simplex, dir);
        }
    }
    else {
        if(SameDirection(ab.Cross(abc), ao)) {
            simplex = {};
            simplex.PushBack(a);
            simplex.PushBack(b);
            return Line(simplex, dir);            
        }
        else {
            if(SameDirection(abc, ao)) {
                dir = abc;
            }
            else {
                simplex = {};
                simplex.PushBack(a);
                simplex.PushBack(c);
                simplex.PushBack(b);
                dir = abc * -1.0f;

            }

        }
    }

    return false;
}

bool GJK::Tetrahedron(Simplex &simplex, Vec3 &dir) {
    
    Vec3 a = simplex.points[0];
    Vec3 b = simplex.points[1];
    Vec3 c = simplex.points[2];
    Vec3 d = simplex.points[3];

    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 ad = d - a;
    Vec3 ao = a * -1.0f;

    Vec3 abc = ab.Cross(ac);
    Vec3 acd = ac.Cross(ad);
    Vec3 adb = ad.Cross(ab);

    if(SameDirection(abc, ao)) {
        simplex = {};
        simplex.PushBack(a);
        simplex.PushBack(b);
        simplex.PushBack(c);
        return Triangle(simplex, dir);
    }
    if(SameDirection(acd, ao)) {
        simplex = {};
        simplex.PushBack(a);
        simplex.PushBack(c);
        simplex.PushBack(d);
        return Triangle(simplex, dir);
    }
    if(SameDirection(adb, ao)) {
        simplex = {};
        simplex.PushBack(a);
        simplex.PushBack(d);
        simplex.PushBack(b);
        return Triangle(simplex, dir);
    }

    return true;
}

bool GJK::SameDirection(Vec3 &dir, Vec3 &ao) {
    return dir.Dot(ao) > 0;
}
