#ifndef _ANIMATION_CMP_H_
#define _ANIMATION_CMP_H_

#include "../common.h"
#include "../animation.h"
#include "base_cmp.h"

struct Entity_;
struct AnimationCMP;
struct PlayerAnimationState_;
struct Input;

struct PlayerAnimationTransition_ {
    bool inTransition = false;
    f32 duration;
    f32 time;
    PlayerAnimationState_ *src;
    PlayerAnimationState_ *des; 

    void Start(PlayerAnimationState_ *src, PlayerAnimationState_ *des, f32 duration);
    void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt);
    bool InProgress();
    bool Finished();
    PlayerAnimationState_ *GetNextState();
};

struct PlayerAnimationState_ {
    virtual void Initialize(AnimationCMP *component) = 0;
    
    virtual PlayerAnimationState_ *Update(Entity_ *entity, Input *input, f32 dt) = 0;
    virtual void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) = 0;
    virtual void Enter(Entity_ *entity) = 0;
    virtual void Exit(Entity_ *entity) = 0;

    void CalculateCurrentAnimationFrame(Entity_ *entity, f32 dt);

    AnimationCMP *anim;
    
};

struct PlayerAnimationIdleState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(Entity_ *entity, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt);
    void Enter(Entity_ *entity) override;
    void Exit(Entity_ *entity) override;
    
    Animation idleAnimation;
};

struct PlayerAnimationWalkState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(Entity_ *entity, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt);
    void Enter(Entity_ *entity) override;
    void Exit(Entity_ *entity) override;

    Animation walkAnimation;
    Animation idleAnimation;
};

struct PlayerAnimationJumpState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;

    PlayerAnimationState_ *Update(Entity_ *entity, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt);
    void Enter(Entity_ *entity) override;
    void Exit(Entity_ *entity) override;
    
    Animation jumpAnimation;
};

struct PlayerAnimationFallState_ : public PlayerAnimationState_ {

    PlayerAnimationFallState_() {
        i32 ImHere = 0;

    }

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(Entity_ *entity, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt);
    void Enter(Entity_ *entity) override;
    void Exit(Entity_ *entity) override;
    
    Animation fallAnimation;
};

#include <stdio.h>

struct AnimationCMP : CMP<AnimationCMP> {

    void Initialize(AnimationClipSet* animationSet_) { 
        animationSet = animationSet_;
        numFinalTransformMatrix = animationSet->skeleton->numJoints;
        idle.Initialize(this);
        walk.Initialize(this);
        jump.Initialize(this);
        fall.Initialize(this);
        state = &idle;
    }

    AnimationClipSet *animationSet;
    PlayerAnimationState_ *state;

    Mat4 *finalTransformMatrix;
    u32 numFinalTransformMatrix;

    PlayerAnimationTransition_ transition;

    PlayerAnimationIdleState_ idle;
    PlayerAnimationWalkState_ walk;
    PlayerAnimationJumpState_ jump;
    PlayerAnimationFallState_ fall;
};


#endif // _ANIMATION_CMP_H_
