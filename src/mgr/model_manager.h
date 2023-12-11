#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_


#include "../asset_manager.h"
#include "../graphics.h"

struct ModelManager : AssetManager<Model> {
    void Load(Model *data, const char *name) override;
    void Unload(Model *data) override;

    void SetTexture(Handle model, char *name);

    inline static ModelManager *Get() { return &modelManager; }
    static ModelManager modelManager;
};

#endif // _MODEL_MANAGER_H_
