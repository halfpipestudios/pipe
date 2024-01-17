#ifndef _ANIMATION_CMP_H_
#define _ANIMATION_CMP_H_

#include "../common.h"
#include "../animation.h"
#include "base_cmp.h"

struct Entity_;
struct AnimationCMP;
struct PlayerAnimationState_;
struct Input;
struct EntityManager;

struct PlayerAnimationTransition_ {
    bool inTransition = false;
    f32 duration;
    f32 time;
    PlayerAnimationState_ *src;
    PlayerAnimationState_ *des; 

    void Start(PlayerAnimationState_ *src, PlayerAnimationState_ *des, f32 duration);
    void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt);
    bool InProgress();
    bool Finished();
    PlayerAnimationState_ *GetNextState();
};

struct PlayerAnimationState_ {
    virtual void Initialize(AnimationCMP *component) = 0;
    
    virtual PlayerAnimationState_ *Update(EntityManager *em, SlotmapKey entityKey, Input *input, f32 dt) = 0;
    virtual void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt) = 0;
    virtual void Enter(SlotmapKey entityKey) = 0;
    virtual void Exit(SlotmapKey entityKey) = 0;

    void CalculateCurrentAnimationFrame(EntityManager *em, SlotmapKey entityKey, f32 dt);

    AnimationCMP *anim;
    
};

struct PlayerAnimationIdleState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(EntityManager *em, SlotmapKey entityKey, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt) override;
    void Enter(SlotmapKey entityKey) override;
    void Exit(SlotmapKey entityKey) override;
    
    Animation idleAnimation;
};

struct PlayerAnimationWalkState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(EntityManager *em, SlotmapKey entityKey, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt) override;
    void Enter(SlotmapKey entityKey) override;
    void Exit(SlotmapKey entityKey) override;

    Animation walkAnimation;
    Animation idleAnimation;
};

struct PlayerAnimationJumpState_ : public PlayerAnimationState_ {

    void Initialize(AnimationCMP *component) override;

    PlayerAnimationState_ *Update(EntityManager *em, SlotmapKey entityKey, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt) override;
    void Enter(SlotmapKey entityKey) override;
    void Exit(SlotmapKey entityKey) override;
    
    Animation jumpAnimation;
};

struct PlayerAnimationFallState_ : public PlayerAnimationState_ {

    PlayerAnimationFallState_() {
        i32 ImHere = 0;

    }

    void Initialize(AnimationCMP *component) override;
    
    PlayerAnimationState_ *Update(EntityManager *em, SlotmapKey entityKey, Input *input, f32 dt);
    void SampleJointPose(JointPose *pose, EntityManager *em, SlotmapKey entityKey, f32 dt) override;
    void Enter(SlotmapKey entityKey) override;
    void Exit(SlotmapKey entityKey) override;
    
    Animation fallAnimation;
};

#include <stdio.h>

struct AnimationCMP : CMP<AnimationCMP> {
    
    AnimationCMP &operator=(AnimationCMP &other) {
        
        id = other.id;
        entityKey = other.entityKey;

        idle = other.idle;
        walk = other.walk;
        jump = other.jump;
        fall = other.fall;

        idle.anim = this;
        walk.anim = this;
        jump.anim = this;
        fall.anim = this;
        state = &idle;

        transition = other.transition;

        return *this;
    }

    void Initialize(AnimationClipSet* animationSet_) { 
        animationSet = animationSet_;
        numFinalTransformMatrix = animationSet->skeleton->numJoints;
        idle.Initialize(this);
        walk.Initialize(this);
        jump.Initialize(this);
        fall.Initialize(this);
        state = &idle;
        state->anim = this;
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

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "player_animation");
        WriteEndObject(s);
    };

};


#endif // _ANIMATION_CMP_H_
