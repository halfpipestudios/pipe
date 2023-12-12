#ifndef _ANIMATION_MANAGER_H_
#define _ANIMATION_MANAGER_H_

#include "asset_manager.h"
#include "animation.h"

struct AnimationManager : AssetManager<AnimationClipSet> {

    void Load(AnimationClipSet *data, const char *name) override;
    void Unload(AnimationClipSet *data) override;

    inline static AnimationManager *Get() { return &animationManager; }
    static AnimationManager animationManager;

};

#endif // _ANIMATION_MANAGER_H_
