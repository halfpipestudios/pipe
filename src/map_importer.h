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

struct Poly
{
    Vertex vertices[256];
    i32 verticesCount;
};

struct HMapHeader {
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
    Vertex *data;
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
    EntitiesInfo GetEntitiesInfo(File file);
    i32 GetTextureCount(File file);
    EntityArray LoadMapFromFile(char *filepath);
    VertexArray VertexArrayCreateFromEntityArray(EntityArray entityArray);
};

#endif
