#ifndef _SHADER_MANAGER_H_
#define _SHADER_MANAGER_H_

#include "asset_manager.h"
#include "graphics.h"

struct VShaderManager : AssetManager<VShader> {
    void Load(VShader *data, const char *name) override;
    void Unload(VShader *data) override;
    
    inline static VShaderManager *Get() { return &vShaderManager; }

    static VShaderManager vShaderManager; 
};

struct FShaderManager : AssetManager<FShader> {
    void Load(FShader *data, const char *name) override;
    void Unload(FShader *data) override;
    
    inline static FShaderManager *Get() { return &fShaderManager; }

    static FShaderManager fShaderManager; 
};

struct GShaderManager : AssetManager<GeometryShader> {
    void Load(GeometryShader *data, const char *name) override;
    void Unload(GeometryShader *data) override;
    
    inline static GShaderManager *Get() { return &gShaderManager; }

    static GShaderManager gShaderManager; 
};

struct GSOShaderManager : AssetManager<GeometryShader> {
    void Load(GeometryShader *data, const char *name) override;
    void Unload(GeometryShader *data) override;
    
    inline static GSOShaderManager *Get() { return &gsoShaderManager; }

    static GSOShaderManager gsoShaderManager; 
};



#endif
