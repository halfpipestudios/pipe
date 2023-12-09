#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "../asset_manager.h"
#include "../graphics.h"


struct TextureManager : AssetManager<TextureBuffer, 256> {

    void Load(TextureBuffer *data, const char *name) override;
    void Unload(TextureBuffer *data) override;

    // NOTE: Singleton
    inline static TextureManager *Get() {
        if(!textureManager) textureManager = new TextureManager();

        return textureManager;
    }
    
    static inline TextureManager *textureManager;

};

#endif // _TEXTURE_MANAGER_H_
