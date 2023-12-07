#include "animation_sys.h"

#include "../platform_manager.h"

template<typename EM>
void AnimationSys<EM>::Update(EM& em, f32 dt) {

    auto& animations = em.GetComponents<AnimationCMP>();
    for(i32 i = 0; i < animations.size; ++i) {

        AnimationCMP *animation = &animations[i];
        Entity_ *entity = animation->entity;

        Input *input = PlatformManager::Get()->GetInput();
        
        PlayerAnimationState_ *newState = animation->state->Update(entity, input, dt);
        if(newState != nullptr) {
            animation->state->Exit(entity);
            newState->Enter(entity);
            animation->state = newState;
        }
    }
}
