#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "common.h"
#include "algebra.h"
#include "allocators.h"

struct Joint {
    char name[MAX_NAME_SIZE];
    i32 parent;
    Mat4 localTransform;
    Mat4 invBindTransform;
};

struct Skeleton {
    char name[MAX_NAME_SIZE];
    Joint *joints;
    u32 numJoints;

    i32 GetJointIndex(const char *name);
    bool JointIsInHierarchy(i32 index, i32 parentIndex);
};

struct JointPose {
    Vec3 position;
    Quat rotation;
    Vec3 scale;
};

struct AnimationSample {
    f32 time_stamp;
    JointPose *localPoses;
};

struct AnimationClip {
    char name[MAX_NAME_SIZE];
    Skeleton *skeleton;
    
    f32 duration;
    AnimationSample *samples;
    u32 numSamples;
};

struct AnimationClipSet {
    Skeleton *skeleton;
    AnimationClip *clips;
    u32 numClips;

    AnimationClip *FindAnimationClipByName(const char *name);
};

struct Animation {
    AnimationClip *clip;

    i32 root;
    f32 time;
    f32 duration;
    bool loop;
    
    void Initialize(AnimationClip *clip, i32 root, bool loop);
    
    void SampleAnimationPose(JointPose *pose, f32 time);
    void SampleNextAnimationPose(JointPose *pose, f32 dt);

    inline bool Finished() { return time >= duration; }

private:

    void SamplePrevAndNextAnimationPose(AnimationSample *prev, AnimationSample *next, f32 time);
};

void JointPoseMixSamples(JointPose *dst, JointPose *a, JointPose *b, u32 numJoints, f32 t);
void CalculateFinalTransformMatrices(JointPose *finalLocalPose, Mat4 *finalTransformMatrices, Skeleton *skeleton);

#endif // _ANIMATION_H_
