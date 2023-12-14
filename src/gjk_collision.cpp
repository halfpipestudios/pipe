#include "gjk_collision.h"
#include <float.h>
#include <stdio.h>

#define MAX_NUM_OF_ITERATIONS 100

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

f32 Sign(f32 value) {
    if(value >= 0.0f) return 1.0f;
    else return -1.0f;
}

Vec3 Cylinder::FindFurthestPoint(Vec3 v) {

    Vec3 w = v - (u.Dot(v) * u);

    Vec3 result = c;

    if(w.LenSq() >= FLT_EPSILON) {
        result += u * Sign(u.Dot(v)) * n + (w.Normalized() * radii);
    }
    else {
        result += u * Sign(u.Dot(v)) * n;
    }

    return result;

}


void Simplex::PushFront(Point point) {
    points[3] = points[2];
    points[2] = points[1];
    points[1] = points[0];
    points[0] = point;
    count = MIN(count + 1, 4);
}

void Simplex::PushBack(Point point) {
    ASSERT(count < 4);
    points[count++] = point;
}

Point GJK::Support(Cylinder *a, Cylinder *b, Vec3 dir) {
    Point result;
    result.a = a->FindFurthestPoint(dir);
    result.b = b->FindFurthestPoint(dir * -1.0f);
    result.p = result.a - result.b;
    return result;
}

Point GJK::Support(ConvexHull *a, Cylinder *b, Vec3 dir) {
    Point result;
    result.a = a->FindFurthestPoint(dir);
    result.b = b->FindFurthestPoint(dir * -1.0f);
    result.p = result.a - result.b;
    return result;
}

Point GJK::Support(ConvexHull *a, ConvexHull *b, Vec3 dir) {
    Point result;
    result.a = a->FindFurthestPoint(dir);
    result.b = b->FindFurthestPoint(dir * -1.0f);
    result.p = result.a - result.b;
    return result;
}


CollisionData GJK::Intersect(ConvexHull *a, ConvexHull *b) {

    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;
    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData = EPA(simplex, a, b);
            return collisionData;
        }

    }
    return {};

}

CollisionData GJK::Intersect(ConvexHull *a, Cylinder *b) {
    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;

    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData = EPA(simplex, a, b);
            return collisionData;
        }

    }
    return {};
}


CollisionData GJK::Intersect(Cylinder *a, Cylinder *b) {
    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;
    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData = EPA(simplex, a, b);
            return collisionData;
        }

    }
    return {};
}


// Intersect Fast
CollisionData GJK::IntersectFast(ConvexHull *a, ConvexHull *b) {

    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;
    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData;
            collisionData.hasCollision = true;
            return collisionData;
        }

    }
    return {};

}

CollisionData GJK::IntersectFast(ConvexHull *a, Cylinder *b) {
    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;

    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData;
            collisionData.hasCollision = true;
            return collisionData;
        }

    }
    return {};
}


CollisionData GJK::IntersectFast(Cylinder *a, Cylinder *b) {
    Point support = Support(a, b, Vec3(1, 0, 0));

    Simplex simplex = {};
    simplex.PushFront(support);

    Vec3 dir = support.p * -1.0f;
    i32 counter = 0;
    while(counter < MAX_NUM_OF_ITERATIONS) {
        counter++;
        support = Support(a, b, dir);

        if(support.p.Dot(dir) <= 0) {
            return {};
        }

        simplex.PushFront(support);

        if(DoSimplex(simplex, dir)) {
            CollisionData collisionData;
            collisionData.hasCollision = true;
            return collisionData;
        }

    }
    return {};
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

    Point a = simplex.points[0];
    Point b = simplex.points[1];

    Vec3 ab = b.p - a.p;
    Vec3 ao = a.p * -1.0f;

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

    Point a = simplex.points[0];
    Point b = simplex.points[1];
    Point c = simplex.points[2];

    Vec3 ab = b.p - a.p;
    Vec3 ac = c.p - a.p;
    Vec3 ao = a.p * -1.0f;

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
    
    Point a = simplex.points[0];
    Point b = simplex.points[1];
    Point c = simplex.points[2];
    Point d = simplex.points[3];

    Vec3 ab = b.p - a.p;
    Vec3 ac = c.p - a.p;
    Vec3 ad = d.p - a.p;
    Vec3 ao = a.p * -1.0f;

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

Plane GJK::GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c) {
    Plane p;
    p.n = ((b - a).Cross(c - a)).Normalized();
    p.d = p.n.Dot(a);
    return p;
}

Vec3 GJK::ProjectPointOntoPlane(Plane &plane, Vec3 &point) {
    f32 distance = point.Dot(plane.n) - plane.d;
    return point - (plane.n * distance);
}

bool GJK::SameDirection(Vec3 &dir, Vec3 &ao) {
    return dir.Dot(ao) > 0;
}

void GJK::Barycentric(Vec3 &a, Vec3 &b, Vec3 &c, Vec3 &p, f32 &u, f32 &v, f32 &w) {
    Vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    f32 d00 = v0.Dot(v0);
    f32 d01 = v0.Dot(v1);
    f32 d11 = v1.Dot(v1);
    f32 d20 = v2.Dot(v0);
    f32 d21 = v2.Dot(v1);
    f32 denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
CollisionData GJK::EPA(Simplex &simplex, ConvexHull *a, ConvexHull *b) {
    Point faces[EPA_MAX_NUM_FACES][4];
    faces[0][0] = simplex.a;
    faces[0][1] = simplex.b;
    faces[0][2] = simplex.c;
    faces[0][3].p = (simplex.b.p - simplex.a.p).Cross(simplex.c.p - simplex.a.p).Normalized(); //ABC
    faces[1][0] = simplex.a;
    faces[1][1] = simplex.c;
    faces[1][2] = simplex.d;
    faces[1][3].p = (simplex.c.p - simplex.a.p).Cross(simplex.d.p - simplex.a.p).Normalized(); //ACD
    faces[2][0] = simplex.a;
    faces[2][1] = simplex.d;
    faces[2][2] = simplex.b;
    faces[2][3].p = (simplex.d.p - simplex.a.p).Cross(simplex.b.p - simplex.a.p).Normalized(); //ADB
    faces[3][0] = simplex.b;
    faces[3][1] = simplex.d;
    faces[3][2] = simplex.c;
    faces[3][3].p = (simplex.d.p - simplex.b.p).Cross(simplex.c.p - simplex.b.p).Normalized(); //BCD
    
    i32 numFaces = 4;
    i32 closestFace;

    for(i32 iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
        // Find face thats closest to origin
        f32 minDist = faces[0][0].p.Dot(faces[0][3].p);
        closestFace = 0;
        for(i32 i = 1; i < numFaces; i++) {
            f32 dist = faces[i][0].p.Dot(faces[i][3].p);
            if(dist < minDist) {
                minDist = dist;
                closestFace = i;
            }
        }

        // search normal to face thats closest to origin
        Vec3 searchDir = faces[closestFace][3].p;
        Point p = Support(a, b, searchDir);

        if(p.p.Dot(searchDir) - minDist < EPA_TOLERANCE) {
            Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
            Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

            f32 u, v, w;
            Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

            Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
            Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
            f32 penetration = (localA - localB).Len();
            
            Vec3 normal = (localA - localB).Normalized();

            CollisionData collisionData;
            collisionData.normal = normal;
            collisionData.penetration = penetration + 0.001f;
            collisionData.hasCollision = true;
            return collisionData;
        }

        Point looseEdges[EPA_MAX_NUM_LOOSE_EDGES][2];
        i32 numLooseEdges = 0;

        // Find all triangle that are facing p
        for(i32 i = 0; i < numFaces; i++) {

            if(faces[i][3].p.Dot(p.p - faces[i][0].p) > 0) {

                for(i32 j = 0; j < 3; j++) {
                    Point currentEdge[2] = {faces[i][j], faces[i][(j + 1) % 3]};
                    bool foundEdge = false;
                    for(i32 k = 0; k < numLooseEdges; k++) {
                        if (looseEdges[k][1].p == currentEdge[0].p && looseEdges[k][0].p == currentEdge[1].p) {
							looseEdges[k][0] = looseEdges[numLooseEdges - 1][0]; //Overwrite current edge
							looseEdges[k][1] = looseEdges[numLooseEdges - 1][1]; //with last edge in list
							numLooseEdges--;
							foundEdge = true;
							k = numLooseEdges; //exit loop because edge can only be shared once
						}
                    }

                    if(!foundEdge) {
                        if(numLooseEdges >= EPA_MAX_NUM_LOOSE_EDGES) break;
					    looseEdges[numLooseEdges][0] = currentEdge[0];
						looseEdges[numLooseEdges][1] = currentEdge[1];
						numLooseEdges++;
                    }
                }

                // Remove triangle i from list
				faces[i][0] = faces[numFaces - 1][0];
				faces[i][1] = faces[numFaces - 1][1];
				faces[i][2] = faces[numFaces - 1][2];
				faces[i][3] = faces[numFaces - 1][3];
				numFaces--;
				i--;
            }
        }

		//Reconstruct polytope with p added
		for (i32 i = 0; i < numLooseEdges; i++)
		{
			// assert(num_faces<EPA_MAX_NUM_FACES);
			if (numFaces >= EPA_MAX_NUM_FACES) break;
			faces[numFaces][0] = looseEdges[i][0];
			faces[numFaces][1] = looseEdges[i][1];
			faces[numFaces][2] = p;
			faces[numFaces][3].p = (looseEdges[i][0].p - looseEdges[i][1].p).Cross(looseEdges[i][0].p - p.p).Normalized();

			//Check for wrong normal to maintain CCW winding
			f32 bias = 0.000001f; //in case dot result is only slightly < 0 (because origin is on face)
			if (faces[numFaces][0].p.Dot(faces[numFaces][3].p) + bias < 0) {
				Point temp = faces[numFaces][0];
				faces[numFaces][0] = faces[numFaces][1];
				faces[numFaces][1] = temp;
				faces[numFaces][3].p = faces[numFaces][3].p * -1.0f;
			}
			numFaces++;
		}
    }


	printf("EPA did not converge\n");

    // search normal to face thats closest to origin
    Vec3 searchDir = faces[closestFace][3].p;

    Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
    Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

    f32 u, v, w;
    Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

    Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
    Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
    f32 penetration = (localA - localB).Len();

    Vec3 normal = (localA - localB).Normalized();

    CollisionData collisionData;
    collisionData.normal = normal;
    collisionData.penetration = penetration + 0.001f;
    collisionData.hasCollision = true;
    return collisionData;
}

CollisionData GJK::EPA(Simplex &simplex, ConvexHull *a, Cylinder *b) {
    Point faces[EPA_MAX_NUM_FACES][4];
    faces[0][0] = simplex.a;
    faces[0][1] = simplex.b;
    faces[0][2] = simplex.c;
    faces[0][3].p = (simplex.b.p - simplex.a.p).Cross(simplex.c.p - simplex.a.p).Normalized(); //ABC
    faces[1][0] = simplex.a;
    faces[1][1] = simplex.c;
    faces[1][2] = simplex.d;
    faces[1][3].p = (simplex.c.p - simplex.a.p).Cross(simplex.d.p - simplex.a.p).Normalized(); //ACD
    faces[2][0] = simplex.a;
    faces[2][1] = simplex.d;
    faces[2][2] = simplex.b;
    faces[2][3].p = (simplex.d.p - simplex.a.p).Cross(simplex.b.p - simplex.a.p).Normalized(); //ADB
    faces[3][0] = simplex.b;
    faces[3][1] = simplex.d;
    faces[3][2] = simplex.c;
    faces[3][3].p = (simplex.d.p - simplex.b.p).Cross(simplex.c.p - simplex.b.p).Normalized(); //BCD
    
    i32 numFaces = 4;
    i32 closestFace;

    for(i32 iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
        // Find face thats closest to origin
        f32 minDist = faces[0][0].p.Dot(faces[0][3].p);
        closestFace = 0;
        for(i32 i = 1; i < numFaces; i++) {
            f32 dist = faces[i][0].p.Dot(faces[i][3].p);
            if(dist < minDist) {
                minDist = dist;
                closestFace = i;
            }
        }

        // search normal to face thats closest to origin
        Vec3 searchDir = faces[closestFace][3].p;
        Point p = Support(a, b, searchDir);

        if(p.p.Dot(searchDir) - minDist < EPA_TOLERANCE) {
            Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
            Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

            f32 u, v, w;
            Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

            Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
            Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
            f32 penetration = (localA - localB).Len();

            Vec3 normal = (localA - localB).Normalized();

            CollisionData collisionData;
            collisionData.normal = normal;
            collisionData.penetration = penetration + 0.001f;
            collisionData.hasCollision = true;
            return collisionData;
        }

        Point looseEdges[EPA_MAX_NUM_LOOSE_EDGES][2];
        i32 numLooseEdges = 0;

        // Find all triangle that are facing p
        for(i32 i = 0; i < numFaces; i++) {

            if(faces[i][3].p.Dot(p.p - faces[i][0].p) > 0) {

                for(i32 j = 0; j < 3; j++) {
                    Point currentEdge[2] = {faces[i][j], faces[i][(j + 1) % 3]};
                    bool foundEdge = false;
                    for(i32 k = 0; k < numLooseEdges; k++) {
                        if (looseEdges[k][1].p == currentEdge[0].p && looseEdges[k][0].p == currentEdge[1].p) {
							looseEdges[k][0] = looseEdges[numLooseEdges - 1][0]; //Overwrite current edge
							looseEdges[k][1] = looseEdges[numLooseEdges - 1][1]; //with last edge in list
							numLooseEdges--;
							foundEdge = true;
							k = numLooseEdges; //exit loop because edge can only be shared once
						}
                    }

                    if(!foundEdge) {
                        if(numLooseEdges >= EPA_MAX_NUM_LOOSE_EDGES) break;
					    looseEdges[numLooseEdges][0] = currentEdge[0];
						looseEdges[numLooseEdges][1] = currentEdge[1];
						numLooseEdges++;
                    }
                }

                // Remove triangle i from list
				faces[i][0] = faces[numFaces - 1][0];
				faces[i][1] = faces[numFaces - 1][1];
				faces[i][2] = faces[numFaces - 1][2];
				faces[i][3] = faces[numFaces - 1][3];
				numFaces--;
				i--;
            }
        }

		//Reconstruct polytope with p added
		for (i32 i = 0; i < numLooseEdges; i++)
		{
			// assert(num_faces<EPA_MAX_NUM_FACES);
			if (numFaces >= EPA_MAX_NUM_FACES) break;
			faces[numFaces][0] = looseEdges[i][0];
			faces[numFaces][1] = looseEdges[i][1];
			faces[numFaces][2] = p;
			faces[numFaces][3].p = (looseEdges[i][0].p - looseEdges[i][1].p).Cross(looseEdges[i][0].p - p.p).Normalized();

			//Check for wrong normal to maintain CCW winding
			f32 bias = 0.000001f; //in case dot result is only slightly < 0 (because origin is on face)
			if (faces[numFaces][0].p.Dot(faces[numFaces][3].p) + bias < 0) {
				Point temp = faces[numFaces][0];
				faces[numFaces][0] = faces[numFaces][1];
				faces[numFaces][1] = temp;
				faces[numFaces][3].p = faces[numFaces][3].p * -1.0f;
			}
			numFaces++;
		}
    }


	printf("EPA did not converge\n");

    // search normal to face thats closest to origin
    Vec3 searchDir = faces[closestFace][3].p;

    Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
    Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

    f32 u, v, w;
    Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

    Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
    Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
    f32 penetration = (localA - localB).Len();

    Vec3 normal = (localA - localB).Normalized();

    CollisionData collisionData;
    collisionData.normal = normal;
    collisionData.penetration = penetration + 0.001f;
    collisionData.hasCollision = true;
    return collisionData;
}

CollisionData GJK::EPA(Simplex &simplex, Cylinder *a, Cylinder *b) {
    Point faces[EPA_MAX_NUM_FACES][4];
    faces[0][0] = simplex.a;
    faces[0][1] = simplex.b;
    faces[0][2] = simplex.c;
    faces[0][3].p = (simplex.b.p - simplex.a.p).Cross(simplex.c.p - simplex.a.p).Normalized(); //ABC
    faces[1][0] = simplex.a;
    faces[1][1] = simplex.c;
    faces[1][2] = simplex.d;
    faces[1][3].p = (simplex.c.p - simplex.a.p).Cross(simplex.d.p - simplex.a.p).Normalized(); //ACD
    faces[2][0] = simplex.a;
    faces[2][1] = simplex.d;
    faces[2][2] = simplex.b;
    faces[2][3].p = (simplex.d.p - simplex.a.p).Cross(simplex.b.p - simplex.a.p).Normalized(); //ADB
    faces[3][0] = simplex.b;
    faces[3][1] = simplex.d;
    faces[3][2] = simplex.c;
    faces[3][3].p = (simplex.d.p - simplex.b.p).Cross(simplex.c.p - simplex.b.p).Normalized(); //BCD
    
    i32 numFaces = 4;
    i32 closestFace;

    for(i32 iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
        // Find face thats closest to origin
        f32 minDist = faces[0][0].p.Dot(faces[0][3].p);
        closestFace = 0;
        for(i32 i = 1; i < numFaces; i++) {
            f32 dist = faces[i][0].p.Dot(faces[i][3].p);
            if(dist < minDist) {
                minDist = dist;
                closestFace = i;
            }
        }

        // search normal to face thats closest to origin
        Vec3 searchDir = faces[closestFace][3].p;
        Point p = Support(a, b, searchDir);

        if(p.p.Dot(searchDir) - minDist < EPA_TOLERANCE) {
            Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
            Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

            f32 u, v, w;
            Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

            Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
            Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
            f32 penetration = (localA - localB).Len();

            Vec3 normal = (localA - localB).Normalized();

            CollisionData collisionData;
            collisionData.normal = normal;
            collisionData.penetration = penetration + 0.001f;
            collisionData.hasCollision = true;
            return collisionData;
        }

        Point looseEdges[EPA_MAX_NUM_LOOSE_EDGES][2];
        i32 numLooseEdges = 0;

        // Find all triangle that are facing p
        for(i32 i = 0; i < numFaces; i++) {

            if(faces[i][3].p.Dot(p.p - faces[i][0].p) > 0) {

                for(i32 j = 0; j < 3; j++) {
                    Point currentEdge[2] = {faces[i][j], faces[i][(j + 1) % 3]};
                    bool foundEdge = false;
                    for(i32 k = 0; k < numLooseEdges; k++) {
                        if (looseEdges[k][1].p == currentEdge[0].p && looseEdges[k][0].p == currentEdge[1].p) {
							looseEdges[k][0] = looseEdges[numLooseEdges - 1][0]; //Overwrite current edge
							looseEdges[k][1] = looseEdges[numLooseEdges - 1][1]; //with last edge in list
							numLooseEdges--;
							foundEdge = true;
							k = numLooseEdges; //exit loop because edge can only be shared once
						}
                    }

                    if(!foundEdge) {
                        if(numLooseEdges >= EPA_MAX_NUM_LOOSE_EDGES) break;
					    looseEdges[numLooseEdges][0] = currentEdge[0];
						looseEdges[numLooseEdges][1] = currentEdge[1];
						numLooseEdges++;
                    }
                }

                // Remove triangle i from list
				faces[i][0] = faces[numFaces - 1][0];
				faces[i][1] = faces[numFaces - 1][1];
				faces[i][2] = faces[numFaces - 1][2];
				faces[i][3] = faces[numFaces - 1][3];
				numFaces--;
				i--;
            }
        }

		//Reconstruct polytope with p added
		for (i32 i = 0; i < numLooseEdges; i++)
		{
			// assert(num_faces<EPA_MAX_NUM_FACES);
			if (numFaces >= EPA_MAX_NUM_FACES) break;
			faces[numFaces][0] = looseEdges[i][0];
			faces[numFaces][1] = looseEdges[i][1];
			faces[numFaces][2] = p;
			faces[numFaces][3].p = (looseEdges[i][0].p - looseEdges[i][1].p).Cross(looseEdges[i][0].p - p.p).Normalized();

			//Check for wrong normal to maintain CCW winding
			f32 bias = 0.000001f; //in case dot result is only slightly < 0 (because origin is on face)
			if (faces[numFaces][0].p.Dot(faces[numFaces][3].p) + bias < 0) {
				Point temp = faces[numFaces][0];
				faces[numFaces][0] = faces[numFaces][1];
				faces[numFaces][1] = temp;
				faces[numFaces][3].p = faces[numFaces][3].p * -1.0f;
			}
			numFaces++;
		}
    }


	printf("EPA did not converge\n");

    // search normal to face thats closest to origin
    Vec3 searchDir = faces[closestFace][3].p;

    Plane closestPlane = GetPlaneFromThreePoints(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p);
    Vec3 projectionPoint = ProjectPointOntoPlane(closestPlane, Vec3());

    f32 u, v, w;
    Barycentric(faces[closestFace][0].p, faces[closestFace][1].p, faces[closestFace][2].p, projectionPoint, u, v, w);

    Vec3 localA = faces[closestFace][0].a * u + faces[closestFace][1].a * v + faces[closestFace][2].a * w;
    Vec3 localB = faces[closestFace][0].b * u + faces[closestFace][1].b * v + faces[closestFace][2].b * w;
    f32 penetration = (localA - localB).Len();

    Vec3 normal = (localA - localB).Normalized();

    CollisionData collisionData;
    collisionData.normal = normal;
    collisionData.penetration = penetration + 0.001f;
    collisionData.hasCollision = true;
    return collisionData;
}
