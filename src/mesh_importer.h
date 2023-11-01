#ifndef _MESH_IMPORTER_H_
#define _MESH_IMPORTER_H_

#include "common.h"

#define TWEEN_MAGIC ((unsigned int)('E'<<24)|('E'<<16)|('W'<<8)|'T')

#define TWEEN_MODEL      (1 << 0)
#define TWEEN_SKELETON   (1 << 1)
#define TWEEN_ANIMATIONS (1 << 2)

#define TWEEN_MAX_NAME_SIZE 256

#define READ_U64(buffer) *((u64 *)buffer); buffer += 8
#define READ_U32(buffer) *((u32 *)buffer); buffer += 4
#define READ_U16(buffer) *((u16 *)buffer); buffer += 2
#define READ_U8(buffer) *((u8 *)buffer); buffer += 1

#define READ_S64(buffer) *((i64 *)buffer); buffer += 8
#define READ_S32(buffer) *((i32 *)buffer); buffer += 4
#define READ_S16(buffer) *((i16 *)buffer); buffer += 2
#define READ_S8(buffer) *((i8 *)buffer); buffer += 1

#define READ_F32(buffer) *((f32 *)buffer); buffer += 4

struct Vertex;
struct SkinVertex;
struct Model;

struct Joint;
struct AnimationSample;
struct Skeleton;
struct AnimationClip;

struct Vec3;
struct Mat4;
struct Quat;

struct TweenImporter {

    void ReadString(u8 **file, char *buffer);
    void ReadVertex(u8 **file, Vertex *vertex);

    void ReadMatrix(u8 **file, Mat4 *matrix);
    void ReadVec3(u8 **file, Vec3 *vector);
    void ReadQuat(u8 **file, Quat *quat);
    
    void ReadJoint(u8 **file, Joint *joint);
    void ReadSample(u8 **file, AnimationSample *sample, u32 num_joints);

    void AddWeightToVertex(SkinVertex *vertex, u32 boneId, f32 weight);


};

struct ModelImporter : public TweenImporter {
    void ReadModelFile(Model *model, u8 *file);
};

struct AnimationImporter : public TweenImporter {
    void ReadSkeletonFile(Skeleton *skeleton, AnimationClip **animations, u32 *num_animations, u8 *file);
};

#endif 
