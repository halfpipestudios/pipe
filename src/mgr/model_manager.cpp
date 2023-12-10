#include "model_manager.h"
#include "texture_manager.h"
#include "../graphics_manager.h"
#include "../model_importer.h"
#include <stdio.h>

ModelManager ModelManager::modelManager;

void ModelManager::Load(Model *data, const char *name) {

    ModelImporter modelImporter;
    static char path[4096];
    sprintf(path, "%s%s", "./data/models/", name); 
    modelImporter.Read(path);
 
    Model *model = &modelImporter.model;

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex; 
        mesh->texture = TextureManager::Get()->GetAsset(mesh->material);
        if (model->type == MODEL_TYPE_ANIMATED) {
            mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer((SkinVertex *)mesh->vertices, mesh->numVertices, sizeof(SkinVertex));
        } else {
            mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer((Vertex *)mesh->vertices, mesh->numVertices, sizeof(Vertex));
        }
        mesh->indexBuffer = GraphicsManager::Get()->CreateIndexBuffer(mesh->indices, mesh->numIndices);
    }

    printf("Model: %s loaded\n", path);

    *data = *model;
}

void ModelManager::Unload(Model *model) {
    printf("Remove model: %llu\n", (u64)model);
    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;
        if(mesh->vertexBuffer) GraphicsManager::Get()->DestroyVertexBuffer(mesh->vertexBuffer);
        if(mesh->indexBuffer) GraphicsManager::Get()->DestroyIndexBuffer(mesh->indexBuffer);
    }
}

