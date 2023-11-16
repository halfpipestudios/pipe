#include "animation.h"
#include "memory_manager.h"
#include "math.h"

#include <string.h>

ObjectAllocator<AnimationState> AnimationSet::animationStateAllocator;

/* -------------------------------------------- */
/*        Skeleton                              */
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
        animationState->time = 0;
        animationState->weight = 0;
        animationState->enable = false;
        animationState->loop = false;
        animationState->root = 0;
        
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

void AnimationSet::Play(const char *name, f32 weight, bool loop) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->time = 0;
    animation->weight = weight;
    animation->enable = true;
    animation->loop = loop;
    animation->smooth = false;
    animation->transitionTime = 0;
}

void AnimationSet::Stop(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->enable = false;
    animation->time = 0;
}

void AnimationSet::Pause(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    animation->enable = false;
}

void AnimationSet::Freeze(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    animation->freeze = true;
}

void AnimationSet::Continue(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    animation->enable = true;
    animation->freeze = false;
}

void AnimationSet::PlaySmooth(const char *name, f32 transitionTime) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->time = 0;
    animation->weight = 1;
    animation->enable = true;
    animation->loop = false;
    animation->smooth = true;
    animation->transitionTime = transitionTime;    
}

void AnimationSet::UpdateWeight(const char *name, f32 weight) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->weight = weight;
}

void AnimationSet::SetRootJoint(const char *name, const char *joint) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    animation->root = skeleton->GetJointIndex(joint);
}

bool AnimationSet::IsAnimationFinish(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    return animation->enable == false;

}

bool AnimationSet::IsFreeze(const char *name) {
    AnimationState *animation = FindAnimationByName(name);
    ASSERT(animation != nullptr);
    return animation->freeze == true;
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

void AnimationSet::Update(f32 dt, Mat4 **finalTransformMatricesOut, u32 *numFinaltrasformMatricesOut) {
    
    Mat4 *finalTransformMatrices = (Mat4 *)MemoryManager::Get()->AllocFrameMemory(sizeof(Mat4)*skeleton->numJoints, 8);
    
    MemoryManager::Get()->BeginTemporalMemory();
    JointPose *finalLocalPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*skeleton->numJoints, 8);

    ZeroFinalLocalPose(finalLocalPose);
    
    AnimationState *state = states;
    while(state != nullptr) {
        if(state->enable) {
            UpdateAnimationState(state, dt, finalLocalPose);
        }
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
    
    // NOTE: update animation time
    if(!state->freeze) {
        state->time += dt;
    }

    if(state->time >= animation->duration) {
        if(state->loop == true) {
            state->time = 0;
        } else {
            state->enable = false;
            return;
        }
    }
    
    // NOTE: update animatnion weight
    if(state->smooth == true) {
        f32 transitionTime = state->transitionTime;
        if(state->time <= transitionTime) {
            state->weight = sinf((PI/2*state->time)/transitionTime);
        } else {
            f32 remainingTime = state->animation->duration - state->time;
            if(remainingTime <= transitionTime) {
                f32 time = transitionTime - remainingTime;
                state->weight = sinf((PI/2*(time+transitionTime))/transitionTime);
            } else {
                state->weight = 1;
            }
        }

    }

    MemoryManager::Get()->BeginTemporalMemory();
    JointPose *intermidiateLocalPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*skeleton->numJoints, 8);

    state->SampleAnimationPose(intermidiateLocalPose);

    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        if(skeleton->JointIsInHierarchy(jointIndex, state->root)) {
            
            JointPose *sample_local_pose = intermidiateLocalPose + jointIndex;
            JointPose *local_pose = finalLocalPose + jointIndex;
            
            local_pose->position = Vec3::Lerp(local_pose->position, sample_local_pose->position, state->weight);
            local_pose->rotation = Quat::Slerp(local_pose->rotation, sample_local_pose->rotation, state->weight);
            local_pose->scale = Vec3::Lerp(local_pose->scale, sample_local_pose->scale, state->weight);
            
            int breakHere = 0;
        }
    }

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

void AnimationSet::ZeroFinalLocalPose(JointPose *finalLocalPose) {
    for(u32 jointIndex = 0; jointIndex < skeleton->numJoints; ++jointIndex) { 
        JointPose *local_pose  = finalLocalPose + jointIndex;
        local_pose->position = Vec3(0, 0, 0);
        local_pose->rotation = Quat(0, 0, 0, 0);
        local_pose->scale = Vec3(0, 0, 0);
    }
}
