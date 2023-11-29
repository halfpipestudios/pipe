#include "animation.h"
#include "memory_manager.h"
#include "math.h"

#include <string.h>

/* -------------------------------------------- */
/*              Skeleton                        */
/* -------------------------------------------- */

i32 Skeleton::GetJointIndex(const char *name) {
    for(u32 jointIndex = 0; jointIndex < numJoints; ++jointIndex) {
        Joint *joint = joints + jointIndex;
        if(strcmp(joint->name, name) == 0) {
            return jointIndex;
        }
    }
    ASSERT(!"Invalid code path");
    return -1;
}

bool Skeleton::JointIsInHierarchy(i32 index, i32 parentIndex) {
    if(index == parentIndex) return true;

    Joint *joint = joints + index;
    while(joint->parent > parentIndex) {
        joint = joints + joint->parent;
    }
    if(joint->parent == parentIndex) return true;
    return false;
}

/* -------------------------------------------- */
/*                 Animation Set                */
/* -------------------------------------------- */

AnimationClip *AnimationClipSet::FindAnimationClipByName(const char *name) {
    for(u32 clipIndex = 0; clipIndex < numClips; ++clipIndex) {
        if(strcmp(clips[clipIndex].name, name) == 0) {
            return clips + clipIndex;
        }
    }
    ASSERT(!"Invalid Code Path!");
    return nullptr;
}

/* -------------------------------------------- */
/*            Animation                         */
/* -------------------------------------------- */

void Animation::Initialize(AnimationClip *clip, i32 root, bool loop) {
    this->clip = clip;
    this->root = root;
    this->time = 0;
    this->duration = clip->duration;
    this->loop = loop;
}

void Animation::Update(f32 dt) {
    
    if(time >= duration) {
        if(loop == true) {
            time = 0;
        }
    }

    if(Finished()) return;

    currentPose = (JointPose *)MemoryManager::Get()->AllocFrameMemory(sizeof(JointPose)*clip->skeleton->numJoints, 8);
    SampleAnimationPose();

    time += dt;
}

void Animation::SamplePrevAndNextAnimationPose(AnimationSample *prev, AnimationSample *next, f32 time) {
    
    u32 nextSampleIndex = 1;
    
    for(u32 sampleIndex = 1; sampleIndex < clip->numSamples; ++sampleIndex) {
        AnimationSample *sample = clip->samples + sampleIndex;
        if(sample->time_stamp > time) {
            nextSampleIndex = sampleIndex;
            break;
        }
    }
    
    u32 prevSampleIndex = nextSampleIndex - 1;
    
    *prev = clip->samples[prevSampleIndex];
    *next = clip->samples[nextSampleIndex];
}

void Animation::SampleAnimationPose() {
    AnimationSample prev, next; 
    SamplePrevAndNextAnimationPose(&prev, &next, time);
    f32 progression = (time - prev.time_stamp) / (next.time_stamp - prev.time_stamp);
    JointPoseMixSamples(currentPose, prev.localPoses, next.localPoses, clip->skeleton->numJoints, progression);
}

void Animation::SampleAnimationPose(f32 time) {
    AnimationSample prev, next; 
    SamplePrevAndNextAnimationPose(&prev, &next, time);
    f32 progression = (time - prev.time_stamp) / (next.time_stamp - prev.time_stamp);
    JointPoseMixSamples(currentPose, prev.localPoses, next.localPoses, clip->skeleton->numJoints, progression);
}

/* -------------------------------------------- */
/*            Utility Functions                 */
/* -------------------------------------------- */

void JointPoseMixSamples(JointPose *dst, JointPose *a, JointPose *b, u32 numJoints, f32 t) {
    for(u32 jointIndex = 0; jointIndex < numJoints; ++jointIndex) {
        dst[jointIndex].position = Vec3::Lerp(a[jointIndex].position, b[jointIndex].position, t);
        dst[jointIndex].scale    = Vec3::Lerp(a[jointIndex].scale, b[jointIndex].scale, t);
        dst[jointIndex].rotation = Quat::Slerp(a[jointIndex].rotation, b[jointIndex].rotation, t);
    }
}

void CalculateFinalTransformMatrices(JointPose *finalLocalPose, Mat4 *finalTransformMatrices, Skeleton *skeleton) {
    // NOTE: Calculate final transformation Matrix
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        Vec3 final_position = finalLocalPose[jointIndex].position;
        Quat final_rotation = finalLocalPose[jointIndex].rotation;
        Vec3 final_scale    = finalLocalPose[jointIndex].scale;
        finalTransformMatrices[jointIndex] = Mat4::Translate(final_position) * final_rotation.ToMat4() * Mat4::Scale(final_scale);
    }

    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) {
        Joint *joint = skeleton->joints + jointIndex;
        if(joint->parent == -1) {
            finalTransformMatrices[jointIndex] = Mat4() * finalTransformMatrices[jointIndex];
        } else {
            ASSERT(joint->parent < (i32)jointIndex);
            finalTransformMatrices[jointIndex] = finalTransformMatrices[joint->parent] * finalTransformMatrices[jointIndex];

        }
    }

    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) {
        Joint *joint = skeleton->joints + jointIndex;
        finalTransformMatrices[jointIndex] = finalTransformMatrices[jointIndex] * joint->invBindTransform;
    }

}


#if 0
/* -------------------------------------------- */
/*        Animation State                       */
/* -------------------------------------------- */

void AnimationState::SamplePrevAndNextAnimationPose(AnimationSample *prev, AnimationSample *next, f32 time) {
    
    u32 nextSampleIndex = 1;
    
    for(u32 sampleIndex = 1; sampleIndex < animation->numSamples; ++sampleIndex) {
        AnimationSample *sample = animation->samples + sampleIndex;
        if(sample->time_stamp > time) {
            nextSampleIndex = sampleIndex;
            break;
        }
    }
    
    u32 prevSampleIndex = nextSampleIndex - 1;
    
    *prev = animation->samples[prevSampleIndex];
    *next = animation->samples[nextSampleIndex];

}

void AnimationState::MixSamples(JointPose *dst, JointPose *a, JointPose *b, f32 t) {
    Skeleton *skeleton = animation->skeleton;
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) {
        dst[jointIndex].position = Vec3::Lerp(a[jointIndex].position, b[jointIndex].position, t);
        dst[jointIndex].scale    = Vec3::Lerp(a[jointIndex].scale, b[jointIndex].scale, t);
        dst[jointIndex].rotation = Quat::Slerp(a[jointIndex].rotation, b[jointIndex].rotation, t);
    }
}

void AnimationState::SampleAnimationPose(JointPose *pose) {
    AnimationSample prev, next; 
    SamplePrevAndNextAnimationPose(&prev, &next, time);
    f32 progression = (time - prev.time_stamp) / (next.time_stamp - prev.time_stamp);
    MixSamples(pose, prev.localPoses, next.localPoses, progression);
}

/* -------------------------------------------- */
/*        Animation Set                         */
/* -------------------------------------------- */

void AnimationSet::Initialize(AnimationClip *animations, u32 numAnimations) {
    
    skeleton = animations[0].skeleton;
    numStates = numAnimations; 

    AnimationState *prev = nullptr;

    for(u32 animationIndex = 0; animationIndex < numAnimations; ++animationIndex) {
        
        AnimationClip *animation = animations + animationIndex;
        AnimationState *animationState = animationStateAllocator.Alloc();

        if(animationIndex == 0) {
            states = animationState;
        }
        
        if(prev != nullptr) {
            prev->next = animationState;
        }
        
        ASSERT(skeleton == animation->skeleton);
        
        animationState->animation = animation;
        animationState->time   = 0;
        animationState->weight = 1;
        animationState->loop   = false;
        animationState->root   = 0;
        animationState->scale  = 0;
        
        prev = animationState;
    }
    
}

void AnimationSet::Terminate(void) {
    AnimationState *state = states;
    while(state != nullptr) {
        AnimationState *toFree = state;
        state = state->next;
        animationStateAllocator.Free(toFree);
    }
}

void AnimationSet::SetWeight(const char *name, f32 weight) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->weight = weight;
}

void AnimationSet::UpdateWeightScale(const char *name, f32 scale) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->scale = scale;
}

void AnimationSet::SetRootJoint(const char *name, const char *joint) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->root = skeleton->GetJointIndex(joint);
}

void AnimationSet::SetLoop(const char *name, bool loop) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->loop = loop;
}

bool AnimationSet::IsAnimationFinish(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    return animation->time >= animation->animation->duration;

}

f32 AnimationSet::GetDuration(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    return animation->animation->duration;
}

f32 AnimationSet::GetTimer(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    return animation->time;
}

static inline void InitializeFinalLocalPose(JointPose *finalLocalPose, Skeleton *skeleton) {
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        JointPose *local_pose = finalLocalPose + jointIndex;
        local_pose->position = Vec3(0, 0, 0);
        local_pose->rotation = Quat(0, 0, 0, 0);
        local_pose->scale = Vec3(0, 0, 0);
    }
}

static inline void AddToFinalLocalPose(AnimationSet *animationSet, AnimationState *state, JointPose *finalLocalPose, JointPose *intermidiateLocalPose, Skeleton *skeleton) {
    if(animationSet->totalWeight < 0.0001f) return;
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        if(skeleton->JointIsInHierarchy(jointIndex, state->root)) {
            JointPose *sample_local_pose = intermidiateLocalPose + jointIndex;
            JointPose *local_pose = finalLocalPose + jointIndex;
            local_pose->position = local_pose->position + (sample_local_pose->position * state->weight * state->scale);
            local_pose->rotation = local_pose->rotation + (sample_local_pose->rotation * state->weight * state->scale);
            local_pose->scale    = local_pose->scale    + (sample_local_pose->scale    * state->weight * state->scale);
        }
    }
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        if(skeleton->JointIsInHierarchy(jointIndex, state->root)) {
            JointPose *local_pose = finalLocalPose + jointIndex;
            local_pose->position = local_pose->position / animationSet->totalWeight;
            local_pose->rotation = local_pose->rotation / animationSet->totalWeight;
            local_pose->scale    = local_pose->scale    / animationSet->totalWeight; 
        }
    }
}

static inline void UpdateAnimationSetTotalWeight(AnimationSet *set) {
    set->totalWeight = 0;
    AnimationState *state = set->states;
    while(state != nullptr) {
        set->totalWeight += (state->weight * state->scale);
        state = state->next;
    }
}

void AnimationSet::Update(f32 dt, Mat4 **finalTransformMatricesOut, u32 *numFinaltrasformMatricesOut) {
    
    Mat4 *finalTransformMatrices = (Mat4 *)MemoryManager::Get()->AllocFrameMemory(sizeof(Mat4)*skeleton->numJoints, 8);
    
    MemoryManager::Get()->BeginTemporalMemory();
    JointPose *finalLocalPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*skeleton->numJoints, 8);

    InitializeFinalLocalPose(finalLocalPose, skeleton);

    UpdateAnimationSetTotalWeight(this);
    
    AnimationState *state = states;
    while(state != nullptr) {
        UpdateAnimationState(state, dt, finalLocalPose);
        state = state->next;
    }

    CalculateFinalTransformMatrices(finalLocalPose, finalTransformMatrices);
    MemoryManager::Get()->EndTemporalMemory();

    *finalTransformMatricesOut = finalTransformMatrices;
    *numFinaltrasformMatricesOut = skeleton->numJoints;

}

void AnimationSet::CalculateFinalTransformMatrices(JointPose *finalLocalPose, Mat4 *finalTransformMatrices) {
    // NOTE: Calculate final transformation Matrix
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        Vec3 final_position = finalLocalPose[jointIndex].position;
        Quat final_rotation = finalLocalPose[jointIndex].rotation;
        Vec3 final_scale    = finalLocalPose[jointIndex].scale;
        finalTransformMatrices[jointIndex] = Mat4::Translate(final_position) * final_rotation.ToMat4() * Mat4::Scale(final_scale);
    }

    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) {
        Joint *joint = skeleton->joints + jointIndex;
        if(joint->parent == -1) {
            finalTransformMatrices[jointIndex] = Mat4() * finalTransformMatrices[jointIndex];
        } else {
            ASSERT(joint->parent < (i32)jointIndex);
            finalTransformMatrices[jointIndex] = finalTransformMatrices[joint->parent] * finalTransformMatrices[jointIndex];

        }
    }

    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) {
        Joint *joint = skeleton->joints + jointIndex;
        finalTransformMatrices[jointIndex] = finalTransformMatrices[jointIndex] * joint->invBindTransform;
    }

}

void AnimationSet::UpdateAnimationState(AnimationState *state, f32 dt, JointPose *finalLocalPose) {
    AnimationClip *animation = state->animation;
    
    state->time += dt;
    
    if(state->time >= animation->duration) {
        if(state->loop == true) {
            state->time = 0;
        } else {
            state->scale = 0;
            return;
        }
    }
    
    MemoryManager::Get()->BeginTemporalMemory();
    JointPose *intermidiateLocalPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*skeleton->numJoints, 8);

    state->SampleAnimationPose(intermidiateLocalPose);
    AddToFinalLocalPose(this, state, finalLocalPose, intermidiateLocalPose, skeleton);
    
    MemoryManager::Get()->EndTemporalMemory();
}

AnimationState *AnimationSet::FindAnimationByName(const char *name) {
    AnimationState *state = states;
    while(state != nullptr) {
        if(strcmp(state->animation->name, name) == 0) {
            return state;
        }
        state = state->next;
    }
    return nullptr;
}

#endif
