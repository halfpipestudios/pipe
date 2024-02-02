#include "shadowmap_manager.h"
#include "graphics_manager.h"

ShadowMapManager ShadowMapManager::shadowMapManager;

void ShadowMapManager::Load(ShadowMap *data, const char *name) {
    static char path[4096];
    sprintf(path, "%s%s", "./data/shadowmaps/", name);
    ShadowMap shadowMap = GraphicsManager::Get()->CreateShadowMap(path);
    *data = shadowMap;
    printf("ShadowMap: %s loaded\n", path);
}

void ShadowMapManager::Unload(ShadowMap *data) {
    GraphicsManager::Get()->DestroyShadowMap(*data);
    printf("remove ShadowMap: %llu\n", (u64)data);
}
