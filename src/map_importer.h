#ifndef _MAP_IMPORTER_H_
#define _MAP_IMPORTER_H_

#include "math.h"
#include "platform_manager.h"
#include "graphics_manager.h"

struct Plane {
    Vec3 n;
    f32 d;
};

struct TextureAxis {
    
    Vec3 u, v;
};

struct Poly {
    VertexMap vertices[256];
    i32 verticesCount;
};

struct MapHeader {
    u64 entityOffset;
    u64 textureOffset;
};

struct EntityHeader {
    u32 faceCount;
    u64 faceSize;
};

struct EntityFace {
    Plane plane;
    TextureAxis textureAxis;
    u32 texture;
};

struct TextureHeader {
    u32 textureWidth;
    u32 textureHeight;
};

struct Entity {
    EntityFace *faces;
    i32 facesCount;
};

struct EntityArray {
    Entity *data;
    u32 count;
};

struct VertexArray {
    VertexMap *data;
    u32 count;
};

struct TexArray {
    Texture *data;
    u32 count;
};

struct Map {
    Entity *entities;
};

struct EntitiesInfo{
    u32 entityCount;
    u32 facesCount;
};

struct MapLoader {

    void LoadMapFromFile(char *filepath);
    EntityArray GetEntities();
    VertexArray GetVertices();
    TexArray GetTextures();

private:

    EntitiesInfo GetEntitiesInfo(File file);
    i32 GetTextureCount(File file);

    void FillPolygonsVertices(Entity *entity, Poly *polygons, i32 count);
    void FillPolygonsUvs(Entity *entity, Poly *polygons, i32 count);
    void OrderPolygonsVertices(Entity *entity,  Poly *polygons, i32 count);
    void RemoveRepeatedVertices(Poly *polygons, i32 count);

    bool GetIntersection(Vec3 n1, Vec3 n2, Vec3 n3, f32 d1, f32 d2, f32 d3, VertexMap *vertex);
    Vec3 GetCenterOfPolygon(Poly *polygon);
    Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c);
    void RemoveVertexAtIndex(Poly *poly, i32 index);

    EntityArray array;
    TexArray texArray;
};

#endif
