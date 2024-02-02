#ifndef _SHADOWMAP_MANAGER_H_
#define _SHADOWMAP_MANAGER_H_

#include "asset_manager.h"
#include "graphics.h"

struct ShadowMapManager : AssetManager<ShadowMap> {
    void Load(ShadowMap *data, const char *name) override;
    void Unload(ShadowMap *data) override;

    inline static ShadowMapManager *Get() { return &shadowMapManager; }

    static ShadowMapManager shadowMapManager;

};

#endif
