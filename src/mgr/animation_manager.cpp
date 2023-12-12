#include "animation_manager.h"
#include "model_importer.h"

AnimationManager AnimationManager::animationManager;

void AnimationManager::Load(AnimationClipSet *data, const char *name) {
    
    AnimationImporter importer;
    static char path[4096];
    sprintf(path, "%s%s", "./data/animations/", name); 
    
    importer.Read(path);
    data->clips    = importer.animations;
    data->numClips = importer.numAnimations;
    data->skeleton = importer.skeleton;

    printf("Animation : %s loaded\n", path);
}

void AnimationManager::Unload(AnimationClipSet *data) {
    printf("remove animation: %llu\n", (u64)data);
}
