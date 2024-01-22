#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "asset_manager.h"
#include "graphics.h"


struct TextureManager : AssetManager<TextureBuffer> {
    void Load(TextureBuffer *data, const char *name) override;
    void Unload(TextureBuffer *data) override;
    
    inline static TextureManager *Get() { return &textureManager; }
    static TextureManager textureManager; 

};

#endif // _TEXTURE_MANAGER_H_
