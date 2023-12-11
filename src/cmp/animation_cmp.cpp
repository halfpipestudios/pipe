#include "animation_cmp.h"
#include "physics_cmp.h"
#include "../entity.h"
#include "../memory_manager.h"

// Player Animaton State ------------------------------------
// ----------------------------------------------------------

void PlayerAnimationState_::CalculateCurrentAnimationFrame(Entity_ *entity, f32 dt) {
    Skeleton *skeleton = anim->animationSet->skeleton;
    ASSERT(skeleton != nullptr);
    
    anim->finalTransformMatrix = (Mat4 *)MemoryManager::Get()->AllocFrameMemory(sizeof(Mat4)*anim->numFinalTransformMatrix, 8);
    
    MemoryManager::Get()->BeginTemporalMemory();
    
    JointPose *pose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose) * skeleton->numJoints, 8); 
    
    if(anim->transition.InProgress()) {
        anim->transition.SampleJointPose(pose, entity, dt);
    } else {
        SampleJointPose(pose, entity, dt);
    }
    
    CalculateFinalTransformMatrices(pose, anim->finalTransformMatrix, skeleton);
    
    MemoryManager::Get()->EndTemporalMemory();

}

// Idle Animation state -------------------------------

void PlayerAnimationIdleState_::Initialize(AnimationCMP *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    idleAnimation.Initialize(set->FindAnimationClipByName("idle"), -1, true);
}

void PlayerAnimationIdleState_::SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) {
    idleAnimation.SampleNextAnimationPose(pose, dt);
}

PlayerAnimationState_ *PlayerAnimationIdleState_::Update(Entity_ *entity, Input *input, f32 dt) {

    PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicsComp != nullptr);
    
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;
    if(!anim->transition.InProgress()) {
        if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < (e*10)) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y > (e*10)) {
            anim->transition.Start(this, &anim->jump, 0.2f);
        }
    }
    
    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;

}

void PlayerAnimationIdleState_::Enter(Entity_ *entity) {
}

void PlayerAnimationIdleState_::Exit(Entity_ *entity) {
}

// Walk Animation state -------------------------------

void PlayerAnimationWalkState_::Initialize(AnimationCMP *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    idleAnimation.Initialize(set->FindAnimationClipByName("idle"), -1, true);
    walkAnimation.Initialize(set->FindAnimationClipByName("walking"), -1, true);
}

void PlayerAnimationWalkState_::SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) {

    PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicsComp != nullptr);
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    
    f32 t = CLAMP(vel2d.Len() * 0.25f, 0, 1);
    u32 numJoints = anim->animationSet->skeleton->numJoints;

    MemoryManager::Get()->BeginTemporalMemory();
    
    JointPose *idlePose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    JointPose *walkPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    
    idleAnimation.SampleNextAnimationPose(idlePose, dt);
    walkAnimation.SampleNextAnimationPose(walkPose, dt);
    JointPoseMixSamples(pose, idlePose, walkPose, numJoints, t);
    
    MemoryManager::Get()->EndTemporalMemory();
}

PlayerAnimationState_ *PlayerAnimationWalkState_::Update(Entity_ *entity, Input *input, f32 dt) {

    PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicsComp != nullptr);

    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;
    if(!anim->transition.InProgress()) {
        if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y > (e*10)) {
            anim->transition.Start(this, &anim->jump, 0.2f);
        } else if(physicsComp->physics.vel.Len() < 0.01f) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < (e*10)) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;

}

void PlayerAnimationWalkState_::Enter(Entity_ *entity) {
}

void PlayerAnimationWalkState_::Exit(Entity_ *entity) {
}

// Jump Animation state -------------------------------

void PlayerAnimationJumpState_::Initialize(AnimationCMP *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    jumpAnimation.Initialize(set->FindAnimationClipByName("jump"), -1, false);
}

void PlayerAnimationJumpState_::SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) {
    
    if(jumpAnimation.time > 0.42f) {
        jumpAnimation.SampleAnimationPose(pose, 0.42f);
    } else {
        jumpAnimation.SampleNextAnimationPose(pose, dt);
    }
}

PlayerAnimationState_ *PlayerAnimationJumpState_::Update(Entity_ *entity, Input *input, f32 dt) {

    PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicsComp != nullptr);
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);

    f32 e = 0.01f;

    if(!anim->transition.InProgress()) {
        if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < (e*10)) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        } else if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(physicsComp->physics.vel.Len() < e) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;
}

void PlayerAnimationJumpState_::Enter(Entity_ *entity) {
}

void PlayerAnimationJumpState_::Exit(Entity_ *entity) {
    jumpAnimation.time = 0;
}

// Fall Animation state -------------------------------

void PlayerAnimationFallState_::Initialize(AnimationCMP *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    fallAnimation.Initialize(set->FindAnimationClipByName("jump"), -1, true);
}

void PlayerAnimationFallState_::SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) {
    fallAnimation.SampleAnimationPose(pose, 0.42f);
}

PlayerAnimationState_ *PlayerAnimationFallState_::Update(Entity_ *entity, Input *input, f32 dt) {

    PhysicsCMP *physicsComp = entity->GetComponent<PhysicsCMP>();
    ASSERT(physicsComp != nullptr);
    
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;

    if(!anim->transition.InProgress()) {
        if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(physicsComp->physics.vel.Len() < e) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);

    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;
}

void PlayerAnimationFallState_::Enter(Entity_ *entity) {
}

void PlayerAnimationFallState_::Exit(Entity_ *entity) {
}


// Player Animaton Transition -------------------------------
// ----------------------------------------------------------

void PlayerAnimationTransition_::Start(PlayerAnimationState_ *src, PlayerAnimationState_ *des, f32 duration) {
    ASSERT(inTransition == false);
    this->inTransition = true;
    this->duration = duration;
    this->time = 0;
    this->src = src;
    this->des = des;
}

void PlayerAnimationTransition_::SampleJointPose(JointPose *pose, Entity_ *entity, f32 dt) {
    
    ASSERT(inTransition == true);
    ASSERT(duration > 0);

    f32 t = time / duration;

    ASSERT(src->anim->animationSet->skeleton->numJoints == des->anim->animationSet->skeleton->numJoints)
    u32 numJoints = src->anim->animationSet->skeleton->numJoints;
    
    MemoryManager::Get()->BeginTemporalMemory();

    JointPose *srcPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    JointPose *desPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    
    src->SampleJointPose(srcPose, entity, dt);
    des->SampleJointPose(desPose, entity, dt);
    JointPoseMixSamples(pose, srcPose, desPose, numJoints, t);

    MemoryManager::Get()->EndTemporalMemory();

    time += dt;

    if(time > duration) inTransition = false;

}

bool PlayerAnimationTransition_::InProgress() {
    return inTransition == true && time <= duration;
}

bool PlayerAnimationTransition_::Finished() {
    return inTransition == false && time > duration;
}

PlayerAnimationState_ *PlayerAnimationTransition_::GetNextState() {
    ASSERT(Finished());
    ASSERT(des != nullptr);
    time = 0;
    return des;
}

