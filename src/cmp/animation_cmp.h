#ifndef _ANIMATION_CMP_H_
#define _ANIMATION_CMP_H_

#include "common.h"
#include "base_cmp.h"
#include "animation.h"
#include "mgr/animation_manager.h"

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
#include <string.h>

#define MAX_ANIMATION_SET_NAME_SIZE 256 

struct AnimationCMP : CMP<AnimationCMP> {

    char animationSetName[MAX_ANIMATION_SET_NAME_SIZE];
    PlayerAnimationState_ *state;

    Mat4 *finalTransformMatrix;
    u32 numFinalTransformMatrix;

    PlayerAnimationTransition_ transition;

    PlayerAnimationIdleState_ idle;
    PlayerAnimationWalkState_ walk;
    PlayerAnimationJumpState_ jump;
    PlayerAnimationFallState_ fall;

    
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
    
    inline AnimationClipSet *GetAnimationSet() { return AnimationManager::Get()->Dereference(AnimationManager::Get()->GetAsset(animationSetName)); }

    void Initialize(char *animationSetName_) { 
        strcpy(animationSetName, animationSetName_);
        AnimationClipSet *animationSet = GetAnimationSet();
        numFinalTransformMatrix = animationSet->skeleton->numJoints;
        idle.Initialize(this);
        walk.Initialize(this);
        jump.Initialize(this);
        fall.Initialize(this);
        state = &idle;
        state->anim = this;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "player_animation");
        Write(s, "animation_set", animationSetName);
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        char animationSetName_[MAX_ANIMATION_SET_NAME_SIZE];
        ReadBeginObject(t, "player_animation");
        Read(t, "animation_set", animationSetName_, MAX_ANIMATION_SET_NAME_SIZE);
        ReadEndObject(t);
        Initialize(animationSetName_);
    };

};


#endif // _ANIMATION_CMP_H_
