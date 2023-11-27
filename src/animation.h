#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "math.h"
#include "common.h"
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

// NOTE: the size of all JointPose array is the number of joints of the parent skeleton
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
    Skeleton *skeleton;
    
    char name[MAX_NAME_SIZE];
    f32 duration;
    AnimationSample *samples;
    u32 numSamples;
};

struct AnimationState {
    AnimationClip *animation;
    
    f32 time;
    f32 weight;
    f32 scale;
    
    bool loop;

    i32 root;
    
    AnimationState *next;

    void SampleAnimationPose(JointPose *pose);

private:

    void SamplePrevAndNextAnimationPose(AnimationSample *prev, AnimationSample *next, f32 time);
    void MixSamples(JointPose *dst, JointPose *a, JointPose *b, f32 t);

};

struct AnimationSet {

    Skeleton *skeleton;
    AnimationState *states;
    u32 numStates;
    
    f32 totalWeight;

    void Initialize(AnimationClip *animations, u32 numAnimations);
    void Terminate(void);
    
    void SetWeight(const char *name, f32 weight);
    void UpdateWeightScale(const char *name, f32 scale);

    f32 GetDuration(const char *name);
    f32 GetTimer(const char *name);

    bool IsAnimationFinish(const char *name);

    void Update(f32 dt, Mat4 **finalTransformMatricesOut, u32 *numFinaltrasformMatricesOut);
    
    void SetRootJoint(const char *name, const char *joint);
    void SetLoop(const char *name, bool loop);

private:
    
    void UpdateAnimationState(AnimationState *state, f32 dt, JointPose *finalLocalPose);
    void CalculateFinalTransformMatrices(JointPose *finalLocalPose, Mat4 *finalTransformMatrices);
    
    AnimationState *FindAnimationByName(const char *name);

    static ObjectAllocator<AnimationState> animationStateAllocator;
};

#endif // _ANIMATION_H_
