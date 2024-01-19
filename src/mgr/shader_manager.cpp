#include "shader_manager.h"
#include "graphics_manager.h"


VShaderManager   VShaderManager::vShaderManager;
FShaderManager   FShaderManager::fShaderManager;
GShaderManager   GShaderManager::gShaderManager;
GSOShaderManager GSOShaderManager::gsoShaderManager;

void VShaderManager::Load(VShader *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/shaders/", name);
    VShader shader = GraphicsManager::Get()->CreateVShader(path);
    *data = shader;
    printf("VShader: %s loaded\n", path);
}

void VShaderManager::Unload(VShader *data) {
    GraphicsManager::Get()->DestroyVShader(*data);
    printf("remove VShader: %llu\n", (u64)data);
}

void FShaderManager::Load(FShader *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/shaders/", name);
    FShader shader = GraphicsManager::Get()->CreateFShader(path);
    *data = shader;
    printf("FShader: %s loaded\n", path);
}

void FShaderManager::Unload(FShader *data) {
    GraphicsManager::Get()->DestroyFShader(*data);
    printf("remove FShader: %llu\n", (u64)data);
}

void GShaderManager::Load(GeometryShader *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/shaders/", name);
    GeometryShader shader = GraphicsManager::Get()->CreateGeometryShader(path);
    *data = shader;
    printf("GShader: %s loaded\n", path);
}

void GShaderManager::Unload(GeometryShader *data) {
    GraphicsManager::Get()->DestroyGeometryShader(*data);
    printf("remove GShader: %llu\n", (u64)data);
}

void GSOShaderManager::Load(GeometryShader *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/shaders/", name);
    GeometryShader shader = GraphicsManager::Get()->CreateGeometryShaderWithStreamOutput(path);
    *data = shader;
    printf("GSOShader: %s loaded\n", path);
}

void GSOShaderManager::Unload(GeometryShader *data) {
    GraphicsManager::Get()->DestroyGeometryShader(*data);
    printf("remove GSOShader: %llu\n", (u64)data);
}
