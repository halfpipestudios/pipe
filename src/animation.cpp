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

void Animation::SampleNextAnimationPose(JointPose *pose, f32 dt) {

    if(time >= duration) {
        if(loop == true) {
            time = 0;
        }
    }

    if(Finished()) return;

    AnimationSample prev, next; 
    SamplePrevAndNextAnimationPose(&prev, &next, time);
    f32 progression = (time - prev.time_stamp) / (next.time_stamp - prev.time_stamp);
    JointPoseMixSamples(pose, prev.localPoses, next.localPoses, clip->skeleton->numJoints, progression);

    time += dt;
}

void Animation::SampleAnimationPose(JointPose *pose, f32 time) {
    AnimationSample prev, next; 
    SamplePrevAndNextAnimationPose(&prev, &next, time);
    f32 progression = (time - prev.time_stamp) / (next.time_stamp - prev.time_stamp);
    JointPoseMixSamples(pose, prev.localPoses, next.localPoses, clip->skeleton->numJoints, progression);
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
