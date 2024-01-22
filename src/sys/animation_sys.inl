#include "animation_sys.h"

#include "../platform_manager.h"

template<typename EM>
void AnimationSys<EM>::Update(EM& em, f32 dt) {

    auto& animations = em.GetComponents<AnimationCMP>();
    for(i32 i = 0; i < animations.size; ++i) {

        AnimationCMP *animation = &animations[i];
        SlotmapKey entity = animation->entityKey;

        Input *input = PlatformManager::Get()->GetInput();
        
        PlayerAnimationState_ *newState = animation->state->Update(&em, entity, input, dt);
        if(newState != nullptr) {
            animation->state->Exit(&em, entity);
            newState->Enter(&em, entity);
            animation->state = newState;
        }
    }
}
