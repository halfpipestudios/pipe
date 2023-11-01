#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "common.h"
#include "graphics.h"
#include "allocators.h"
#include "math.h"

#define MAX_NAME_SIZE 256
#define MAX_FINAL_BONE_MATRICES 100

#define BONE_INFLUENCE 4 
struct SkinVertex : public Vertex {
    i32 boneId[BONE_INFLUENCE];
    f32 weight[BONE_INFLUENCE];
};

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

struct SkinMesh : public Mesh {
    SkinVertex *vertices;
    u32 numVertices;
    
};

struct SkinModel : public Model {
    Skeleton *skeleton;
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
    
    f32 transitionTime;

    bool enable;
    bool loop;
    bool smooth;

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

    void Initialize(AnimationClip *animations, u32 numAnimations);
    void Terminate(void);
    
    void Play(const char *name, f32 weight, bool loop);
    void PlaySmooth(const char *name, f32 transitionTime);
    void Stop(const char *name);
    void UpdateWeight(const char *name, f32 weight);
    bool IsAnimationFinish(const char *name);

    void Update(f32 dt);
    
    void SetRootJoint(const char *name, const char *joint);

private:
    
    void UpdateAnimationState(AnimationState *state, f32 dt, JointPose *finalLocalPose);
    void ZeroFinalLocalPose(JointPose *finalLocalPose);
    void CalculateFinalTransformMatrices(JointPose *finalLocalPose, Mat4 *finalTransformMatrices);
    
    AnimationState *FindAnimationByName(const char *name);

    static ObjectAllocator<AnimationState> animationStateAllocator;
};

#endif // _ANIMATION_H_
