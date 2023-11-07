#include "map_importer.h"
#include "memory_manager.h"
#include "gjk_collision.h"

// TODO: FIX AND REMOVE !!!! ....
#include <vector>

MapImporter::EntitiesInfo MapImporter::GetEntitiesInfo(File file) {
    EntitiesInfo info = {};

    u8 *data = (u8 *)file.data;
    MapHeader *mapheader = (MapHeader *)data;
    u8 *entityChunk = data + mapheader->entityOffset;
    u8 *textureChunk = data + mapheader->textureOffset;

    u8 *currentEntity = entityChunk;
    while(currentEntity < textureChunk) {
        EntityHeader *entityHeader = (EntityHeader *)currentEntity;
        
        info.entityCount++;
        info.facesCount += entityHeader->faceCount;

        currentEntity += (entityHeader->faceCount * entityHeader->faceSize) + sizeof(EntityHeader);
    }   

    return info;
}

i32 MapImporter::GetTextureCount(File file) {
    i32 count = 0;

    u8 *data = (u8 *)file.data;

    u8 *eof = data + file.size;

    MapHeader *mapheader = (MapHeader *)data;
    u8 *textureChunk = data + mapheader->textureOffset;

    u8 *currentTexture = textureChunk;
    while(currentTexture < eof) {
        TextureHeader *textureHeader = (TextureHeader *)currentTexture;
        currentTexture += sizeof(TextureHeader);

        count++;

        currentTexture += sizeof(u32) * textureHeader->textureWidth * textureHeader->textureHeight;
    }

    return count;
}

void MapImporter::LoadMapFromFile(char *filepath) {

    MemoryManager::Get()->BeginTemporalMemory();
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(filepath);


    EntitiesInfo entityInfo = GetEntitiesInfo(file);
    i32 texCount = GetTextureCount(file);

    u8 *data = (u8 *)file.data;
    u8 *eof = data + file.size;

    MapHeader *mapheader = (MapHeader *)data;
    u8 *entityChunk = data + mapheader->entityOffset;
    u8 *textureChunk = data + mapheader->textureOffset;

    entityArray = {};
    entityArray.data = (Entity *)MemoryManager::Get()->AllocStaticMemory(entityInfo.entityCount * sizeof(Entity), 1);
    entityArray.count = entityInfo.entityCount;

    i32 index = 0;
    u8 *currentEntity = entityChunk;
    while(currentEntity < textureChunk) {
        EntityHeader *entityHeader = (EntityHeader *)currentEntity;
        u8 *faces = currentEntity + sizeof(EntityHeader);

        Entity *entity = &entityArray.data[index++];
        entity->faces = (EntityFace *)MemoryManager::Get()->AllocStaticMemory(entityHeader->faceCount * entityHeader->faceSize, 1);
        entity->facesCount = entityHeader->faceCount;

        for(i32 i = 0; i < entityHeader->faceCount; ++i) {
            EntityFace *entityFace = (EntityFace *)(faces + (entityHeader->faceSize * i));
            entity->faces[i] = *entityFace;
        }

        currentEntity += (entityHeader->faceCount * entityHeader->faceSize) + sizeof(EntityHeader);
    }


    textureArray.count = texCount;
    textureArray.data = (Texture *)MemoryManager::Get()->AllocStaticMemory(texCount * sizeof(Texture), 1);

    index = 0;
    u8 *currentTexture = textureChunk;
    while(currentTexture < eof) {
        TextureHeader *textureHeader = (TextureHeader *)currentTexture;
        currentTexture += sizeof(TextureHeader);

        u64 textureSize = sizeof(u32) * textureHeader->textureWidth * textureHeader->textureHeight;

        Texture *texture = &textureArray.data[index++];
        texture->w = textureHeader->textureWidth;
        texture->h = textureHeader->textureHeight;
        texture->pixels = (u32 *)MemoryManager::Get()->AllocStaticMemory(textureSize, 1);
        memcpy(texture->pixels, currentTexture, textureSize);

        currentTexture += textureSize;
    }

    MemoryManager::Get()->EndTemporalMemory();


    LoadVertexData();

}

MapImporter::EntityArray MapImporter::GetEntities() {
    return entityArray;
}


MapImporter::TextureArray MapImporter::GetTextures() {
    return textureArray;
}


MapImporter::ConvexHullArray MapImporter::GetConvexHulls() {
    return convexHullArray;
}

MapImporter::VertexArray MapImporter::GetVertices() {
    return vertexArray;
}



void MapImporter::LoadVertexData() {
    std::vector<VertexMap> vertices;

    convexHullArray.data = (ConvexHull *)MemoryManager::Get()->AllocStaticMemory(sizeof(ConvexHull) * entityArray.count, 1);
    convexHullArray.count = entityArray.count;

    for(i32 entityIndex = 0; entityIndex < entityArray.count; ++entityIndex) {
        
        MemoryManager::Get()->BeginTemporalMemory();

        Entity *entity = &entityArray.data[entityIndex];
        i32  planesCount = entity->facesCount;
        
        Poly *polygons = (Poly *)MemoryManager::Get()->AllocTemporalMemory(sizeof(Poly) * planesCount, 1);
        memset(polygons, 0, sizeof(Poly) * planesCount);

        FillPolygonsVertices(entity, polygons, planesCount);
        FillPolygonsUvs(entity, polygons, planesCount);
        OrderPolygonsVertices(entity, polygons, planesCount);
        RemoveRepeatedVertices(polygons, planesCount);

        for(i32 i = 0; i < planesCount; ++i) {
            Poly *polyD = &polygons[i];
            for(i32 j = 0; j < polyD->verticesCount - 2; ++j) {
                VertexMap a = polyD->vertices[0];
                VertexMap b = polyD->vertices[j + 1];
                VertexMap c = polyD->vertices[j + 2];
                vertices.push_back(a);
                vertices.push_back(b);
                vertices.push_back(c);
            }
        }

        MemoryManager::Get()->EndTemporalMemory();

        i32 verticesCount = 0;
        for(i32 i = 0; i < planesCount; ++i) {
            Poly *polyD = &polygons[i];
            for(i32 j = 0; j < polyD->verticesCount; ++j) {
                verticesCount++;
            }
        }

        convexHullArray.data[entityIndex].points = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(sizeof(Vec3) * verticesCount, 1);
        convexHullArray.data[entityIndex].count = verticesCount;

        convexHullArray.data[entityIndex].count = 0;
        for(i32 i = 0; i < planesCount; ++i) {
            Poly *polyD = &polygons[i];
            for(i32 j = 0; j < polyD->verticesCount; ++j) {
                f32 scale = 1.0f/32.0f;
                convexHullArray.data[entityIndex].points[convexHullArray.data[entityIndex].count++] =
                    Mat4::TransformPoint(Mat4::Scale(scale, scale, scale), polyD->vertices[j].pos);
            }
        }

    }


    vertexArray.count = vertices.size();
    vertexArray.data = (VertexMap *)MemoryManager::Get()->AllocStaticMemory(vertices.size() * sizeof(VertexMap), 1);
    memcpy(vertexArray.data, vertices.data(), vertices.size() * sizeof(VertexMap));

}

#define ILEGAL_EPSILON 0.01f
void MapImporter::FillPolygonsVertices(Entity *entity, Poly *polygons, i32 count) {
    for(i32 i = 0; i < count - 2; ++i) {
    for(i32 j = i; j < count - 1; ++j) {
    for(i32 k = j; k < count - 0; ++k) {
        if(i != j && i != k && j != k) {
            Plane a = entity->faces[i].plane;
            Plane b = entity->faces[j].plane;
            Plane c = entity->faces[k].plane;

            VertexMap vertex = {};
            if(GetIntersection(a.n, b.n, c.n, a.d, b.d, c.d, &vertex)) {
                bool illegal = false;
                for(i32 m = 0; m < count; ++m) {
                    Plane plane = entity->faces[m].plane;
                    f32 dot = plane.n.Dot(vertex.pos);
                    f32 d = plane.d;
                    f32 dist = dot - d;
                    if(dist > ILEGAL_EPSILON) {
                        illegal = true;
                    }
                }
                if(illegal == false) {
                    VertexMap iVert = vertex; iVert.nor = entity->faces[i].plane.n;
                    VertexMap jVert = vertex; jVert.nor = entity->faces[j].plane.n;
                    VertexMap kVert = vertex; kVert.nor = entity->faces[k].plane.n;
                    polygons[i].vertices[polygons[i].verticesCount++] = iVert;
                    polygons[j].vertices[polygons[j].verticesCount++] = jVert;
                    polygons[k].vertices[polygons[k].verticesCount++] = kVert;
                }
            }
        }
    }}}
}

void MapImporter::FillPolygonsUvs(Entity *entity, Poly *polygons, i32 count) {
    for(i32 i = 0; i < count; ++i) {
        TextureAxis texAxis = entity->faces[i].textureAxis;
        u32 texture = entity->faces[i].texture;

        Poly *polyD = &polygons[i];
        Vec3 center = GetCenterOfPolygon(polyD);
        for(i32 j = 0; j < polyD->verticesCount; ++j)
        {
            polyD->vertices[j].tex = texture;

            f32 u, v;
            u = texAxis.u.Dot(polyD->vertices[j].pos);
            u = u / 64.0f;

            v = texAxis.v.Dot(polyD->vertices[j].pos);
            v = v / 64.0f;

            polyD->vertices[j].uv = { u, v };
        }
    }
}



void MapImporter::OrderPolygonsVertices(Entity *entity, Poly *polygons, i32 count) {
    // order the vertices in the polygons
    for(i32 p = 0; p < count; ++p) {
        Plane polygonPlane = entity->faces[p].plane; 
        Poly *polygon = &polygons[p];

        ASSERT(polygon->verticesCount >= 3);

        Vec3 center = GetCenterOfPolygon(polygon);

        
        for(i32 n = 0; n <= polygon->verticesCount - 3; ++n) {
            Vec3 a = (polygon->vertices[n].pos - center).Normalized();
            Plane p = GetPlaneFromThreePoints(polygon->vertices[n].pos,
                                              center, center + polygonPlane.n);

            f32 smallestAngle = -1;
            i32 smallest = -1;

            for(i32 m = n + 1; m <= polygon->verticesCount - 1; ++m) {
                VertexMap vertex = polygon->vertices[m];
                if((p.n.Dot(vertex.pos) - p.d) > EPSILON) {
                    Vec3 b = (vertex.pos - center).Normalized();
                    f32 angle = a.Dot(b);
                    if(angle > smallestAngle) {
                        smallestAngle = angle;
                        smallest = m;
                    }
                }
            }

            if(smallest >= 0) {
                VertexMap tmp = polygon->vertices[n + 1];
                polygon->vertices[n + 1] = polygon->vertices[smallest];
                polygon->vertices[smallest] = tmp;
            }
        }
    }
}


void MapImporter::RemoveRepeatedVertices(Poly *polygons, i32 count) {
    // remove repeted vertex
    for(i32 j = 0; j < count; ++j) {
        Poly *poly = &polygons[j];
        for(i32 i = 0; i < poly->verticesCount; ++i) {
            VertexMap a = poly->vertices[i];
            for(i32 k = 0; k < poly->verticesCount; ++k) {
                VertexMap b = poly->vertices[k];
                if(i != k) {
                    // remove the vertex
                    if(a.pos == b.pos) {
                        RemoveVertexAtIndex(poly, k);
                        k--;
                    }
                }
            }
        }
    }
}

bool MapImporter::GetIntersection(Vec3 n1, Vec3 n2, Vec3 n3, f32 d1, f32 d2, f32 d3, VertexMap *vertex) {
    Vec3 u = n2.Cross(n3);
    f32 denom = n1.Dot(u);
    if(fabsf(denom) < FLT_EPSILON) return false;
    Vec3 pos = (d1 * u + n1.Cross(d3 * n2 - d2 * n3)) / denom;
    Vec4 col = {0.9, 0.7, 1, 1.0f};
    *vertex = {pos, {}, {}, 0};
    return true;
}

Vec3 MapImporter::GetCenterOfPolygon(Poly *polygon) {
    Vec3 center = {};
    for(i32 i = 0; i < polygon->verticesCount; ++i) {
        center = center + polygon->vertices[i].pos;
    }
    center = center / polygon->verticesCount;
    return center;
}

Plane MapImporter::GetPlaneFromThreePoints(Vec3 a, Vec3 b, Vec3 c) {
    Plane p;
    p.n = ((b - a).Cross(c - a)).Normalized();
    p.d = p.n.Dot(a);
    return p;
}

void MapImporter::RemoveVertexAtIndex(Poly *poly, i32 index) {
    for(i32 i = index; i < (poly->verticesCount - 1); ++i) {
        poly->vertices[i] = poly->vertices[i + 1];
    }
    poly->verticesCount--;
}
