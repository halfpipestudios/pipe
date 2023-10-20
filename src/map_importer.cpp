#include "map_importer.h"
#include "memory_manager.h"

EntitiesInfo MapLoader::GetEntitiesInfo(File file) {

    EntitiesInfo info = {};

    u8 *data = (u8 *)file.data;
    HMapHeader *hMapheader = (HMapHeader *)data;
    u8 *entityChunk = data + hMapheader->entityOffset;
    u8 *textureChunk = data + hMapheader->textureOffset;

    u8 *currentEntity = entityChunk;
    while(currentEntity < textureChunk) {
        EntityHeader *entityHeader = (EntityHeader *)currentEntity;
        
        info.entityCount++;
        info.facesCount += entityHeader->faceCount;

        currentEntity += (entityHeader->faceCount * entityHeader->faceSize) + sizeof(EntityHeader);
    }   

    return info;
}

i32 MapLoader::GetTextureCount(File file) {
    i32 count = 0;


    u8 *data = (u8 *)file.data;


    u8 *eof = data + file.size;

    HMapHeader *hMapheader = (HMapHeader *)data;
    u8 *textureChunk = data + hMapheader->textureOffset;

    u8 *currentTexture = textureChunk;
    while(currentTexture < eof)
    {
        TextureHeader *textureHeader = (TextureHeader *)currentTexture;
        currentTexture += sizeof(TextureHeader);

        count++;

        currentTexture += sizeof(u32) * textureHeader->textureWidth * textureHeader->textureHeight;
    }

    return count;
}

EntityArray MapLoader::LoadMapFromFile(char *filepath) {

    MemoryManager::Get()->BeginTemporalMemory();
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(filepath);


    EntitiesInfo entityInfo = GetEntitiesInfo(file);
    i32 textureCount = GetTextureCount(file);

    u8 *data = (u8 *)file.data;

    HMapHeader *hMapheader = (HMapHeader *)data;
    u8 *entityChunk = data + hMapheader->entityOffset;
    u8 *textureChunk = data + hMapheader->textureOffset;

    EntityArray array = {};
    array.data = (Entity *)MemoryManager::Get()->AllocStaticMemory(entityInfo.entityCount * sizeof(Entity), 1);
    array.count = entityInfo.entityCount;

    i32 index = 0;
    u8 *currentEntity = entityChunk;
    while(currentEntity < textureChunk) {
        EntityHeader *entityHeader = (EntityHeader *)currentEntity;
        u8 *faces = currentEntity + sizeof(EntityHeader);

        Entity *entity = &array.data[index++];
        entity->faces = (EntityFace *)MemoryManager::Get()->AllocStaticMemory(entityHeader->faceCount * entityHeader->faceSize, 1);
        entity->facesCount = entityHeader->faceCount;

        for(i32 i = 0; i < entityHeader->faceCount; ++i) {
            EntityFace *entityFace = (EntityFace *)(faces + (entityHeader->faceSize * i));
            entity->faces[i] = *entityFace;
        }

        currentEntity += (entityHeader->faceCount * entityHeader->faceSize) + sizeof(EntityHeader);
    }

    MemoryManager::Get()->EndTemporalMemory();

    return array;
}

// TODO: FIX AND REMOVE !!!! ....
#include <math.h>
#include <float.h>
#include "vector"


#define EPSILON 0.0001f
#define VEC_EPSILON 0.000001f

Vec3 operator*(f32 val, Vec3 vec) {
    Vec3 result;
    result.x = vec.x * val;
    result.y = vec.y * val;
    result.z = vec.z * val;
    return result;
}

bool operator==(Vec3 a, Vec3 b) {
    Vec3 diff = {a - b};
    return  diff.LenSq() < VEC_EPSILON;
}

static bool GetIntersection(Vec3 n1, Vec3 n2, Vec3 n3, f32 d1, f32 d2, f32 d3, Vertex *vertex)
{
    Vec3 u = n2.Cross(n3);
    f32 denom = n1.Dot(u);
    if(fabsf(denom) < FLT_EPSILON) return false;
    Vec3 pos = (d1 * u + n1.Cross(d3 * n2 - d2 * n3)) / denom;
    Vec4 col = {0.9, 0.7, 1, 1.0f};
    *vertex = {pos, {}, {}};
    return true;
}

static Vec3 GetCenterOfPolygon(Poly *polygon)
{
    Vec3 center = {};
    for(i32 i = 0; i < polygon->verticesCount; ++i)
    {
        center = center + polygon->vertices[i].pos;
    }
    center = center / polygon->verticesCount;
    return center;
}

static Plane GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c)
{
    Plane p;
    p.n = ((b - a).Cross(c - a)).Normalized();
    p.d = p.n.Dot(a);
    return p;
}

static void RemoveVertexAtIndex(Poly *poly, i32 index)
{
    for(i32 i = index; i < (poly->verticesCount - 1); ++i)
    {
        poly->vertices[i] = poly->vertices[i + 1];
    }
    poly->verticesCount--;
}

VertexArray MapLoader::VertexArrayCreateFromEntityArray(EntityArray entityArray)
{
    std::vector<Vertex> vertices;

    for(i32 entityIndex = 0; entityIndex < entityArray.count; ++entityIndex) {
        
        std::vector<Poly> polygons;
        Entity *entity = &entityArray.data[entityIndex];
        i32  planesCount = entity->facesCount;
        polygons.resize(planesCount);
        for(i32 i = 0; i < planesCount - 2; ++i) {
        for(i32 j = i; j < planesCount - 1; ++j) {
        for(i32 k = j; k < planesCount - 0; ++k) {

            if(i != j && i != k && j != k)
            {
                Plane a = entity->faces[i].plane;
                Plane b = entity->faces[j].plane;
                Plane c = entity->faces[k].plane;

                Vertex vertex = {};
                if(GetIntersection(a.n, b.n, c.n, a.d, b.d, c.d, &vertex))
                {
                    bool illegal = false;
                    for(i32 m = 0; m < planesCount; ++m)
                    {
                        Plane plane = entity->faces[m].plane;
                        f32 dot = plane.n.Dot(vertex.pos);
                        f32 d = plane.d;
                        f32 dist = dot - d;
                        if(dist > EPSILON)
                        {
                            illegal = true;
                        }
                    }
                    if(illegal == false)
                    {
                        Vertex iVert = vertex; iVert.nor = entity->faces[i].plane.n;
                        Vertex jVert = vertex; jVert.nor = entity->faces[j].plane.n;
                        Vertex kVert = vertex; kVert.nor = entity->faces[k].plane.n;
                        polygons[i].vertices[polygons[i].verticesCount++] = iVert;
                        polygons[j].vertices[polygons[j].verticesCount++] = jVert;
                        polygons[k].vertices[polygons[k].verticesCount++] = kVert;
                    }
                }
            }

        }}}

        // order the vertices in the polygons
        for(i32 p = 0; p < planesCount; ++p)
        {
            Plane polygonPlane = entity->faces[p].plane; 
            Poly *polygon = &polygons[p];

            ASSERT(polygon->verticesCount >= 3);

            Vec3 center = GetCenterOfPolygon(polygon);

            
            for(i32 n = 0; n <= polygon->verticesCount - 3; ++n)
            {
                Vec3 a = (polygon->vertices[n].pos - center).Normalized();
                Plane p = GetPlaneFromThreePoints(polygon->vertices[n].pos,
                                                  center, center + polygonPlane.n);

                f32 smallestAngle = -1;
                i32 smallest = -1;

                for(i32 m = n + 1; m <= polygon->verticesCount - 1; ++m)
                {
                    Vertex vertex = polygon->vertices[m];
                    if((p.n.Dot(vertex.pos) - p.d) > EPSILON)
                    {
                        Vec3 b = (vertex.pos - center).Normalized();
                        f32 angle = a.Dot(b);
                        if(angle > smallestAngle)
                        {
                            smallestAngle = angle;
                            smallest = m;
                        }
                    }
                }

                if(smallest >= 0)
                {
                    Vertex tmp = polygon->vertices[n + 1];
                    polygon->vertices[n + 1] = polygon->vertices[smallest];
                    polygon->vertices[smallest] = tmp;
                }
            }
        }

        // remove repeted vertex
        for(i32 j = 0; j < polygons.size(); ++j)
        {
            Poly *poly = &polygons[j];
            for(i32 i = 0; i < poly->verticesCount; ++i)
            {
                Vertex a = poly->vertices[i];
                for(i32 k = 0; k < poly->verticesCount; ++k)
                {
                    Vertex b = poly->vertices[k];
                    if(i != k)
                    {
                        // remove the vertex
                        if(a.pos == b.pos)
                        {
                            RemoveVertexAtIndex(poly, k);
                            k--;
                        }
                    }
                }
            }
        }

        for(i32 i = 0; i < polygons.size(); ++i)
        {
            Poly *polyD = &polygons[i];
            for(i32 j = 0; j < polyD->verticesCount - 2; ++j)
            {
                Vertex a = polyD->vertices[0];
                Vertex b = polyD->vertices[j + 1];
                Vertex c = polyD->vertices[j + 2];
                vertices.push_back(a);
                vertices.push_back(b);
                vertices.push_back(c);
            }
        }
    }


    VertexArray vertexArray = {};
    vertexArray.count = vertices.size();
    vertexArray.data = (Vertex *)MemoryManager::Get()->AllocStaticMemory(vertices.size() * sizeof(Vertex), 1);

    memcpy(vertexArray.data, vertices.data(), vertices.size() * sizeof(Vertex));

    return vertexArray;


}
