#include "model_importer.h"

#include "platform_manager.h"
#include "memory_manager.h"
#include "animation.h"
#include "graphics.h"

#include <stdio.h>
#include <string.h>

void TweenImporter::ReadString(u8 **file, char *buffer) {
    u32 len = READ_U32(*file);
    if(len > MAX_NAME_SIZE) {
        len = MAX_NAME_SIZE;
    }
    memcpy(buffer, *file, len);
    buffer[len] = '\0';
    
    *file += len;
}

void TweenImporter::ReadVertex(u8 **file, Vertex *vertex) {
    // NOTE: Read position
    vertex->pos.x = READ_F32(*file);
    vertex->pos.y = READ_F32(*file);
    vertex->pos.z = READ_F32(*file);

    // NOTE: Skip normals for now
    vertex->nor.x = READ_F32(*file);
    vertex->nor.y = READ_F32(*file);
    vertex->nor.z = READ_F32(*file);
    
    // NOTE: Read texcoords
    vertex->uv.x = READ_F32(*file);
    vertex->uv.y = READ_F32(*file);
}

void TweenImporter::ReadMatrix(u8 **file, Mat4 *matrix) {
    matrix->v[0] = READ_F32(*file);
    matrix->v[1] = READ_F32(*file);
    matrix->v[2] = READ_F32(*file);
    matrix->v[3] = READ_F32(*file);
    
    matrix->v[4] = READ_F32(*file);
    matrix->v[5] = READ_F32(*file);
    matrix->v[6] = READ_F32(*file);
    matrix->v[7] = READ_F32(*file);
    
    matrix->v[8] = READ_F32(*file);
    matrix->v[9] = READ_F32(*file);
    matrix->v[10] = READ_F32(*file);
    matrix->v[11] = READ_F32(*file);
    
    matrix->v[12] = READ_F32(*file);
    matrix->v[13] = READ_F32(*file);
    matrix->v[14] = READ_F32(*file);
    matrix->v[15] = READ_F32(*file);
}

void TweenImporter::ReadVec3(u8 **file, Vec3 *vector) {
    vector->x = READ_F32(*file);
    vector->y = READ_F32(*file);
    vector->z = READ_F32(*file);
}

void TweenImporter::ReadQuat(u8 **file, Quat *quat) {
    quat->w = READ_F32(*file);
    quat->x = READ_F32(*file);
    quat->y = READ_F32(*file);
    quat->z = READ_F32(*file);
}

void TweenImporter::AddWeightToVertex(SkinVertex *vertex, u32 boneId, f32 weight) {
    
    for(u32 i = 0; i < MAX_BONES_INFLUENCE; ++i) {
        if(vertex->boneIds[i] < 0) {
            vertex->boneIds[i] = boneId;
            vertex->weights[i] = weight;
            return;
        }
    }
    
    // NOTE: Vertex weights is full, find the smaller weight and check if we need to add the new one 
    
    f32 minWeight = vertex->weights[0];
    u32 minIndex = 0;
    ASSERT(vertex->boneIds[minIndex] >= 0);

    for(u32 i = 1; i < MAX_BONES_INFLUENCE; ++i) {
        ASSERT(vertex->boneIds[i] >= 0);
        if(vertex->weights[i] < minWeight) {
            minWeight = vertex->weights[i];
            minIndex = i;
        }
    }
    
    if(minWeight < weight) {
        vertex->boneIds[minIndex] = boneId;
        vertex->weights[minIndex] = weight;
    }

}

void TweenImporter::NormalizeVertexWeigths(SkinVertex *vertex) {
    f32 totalSum = 0;
    for(u32 i = 0; i < MAX_BONES_INFLUENCE; ++i) {
        if(vertex->boneIds[i] >= 0) {
            totalSum += vertex->weights[i];
        }
    }

    for(u32 i = 0; i < MAX_BONES_INFLUENCE; ++i) {
        if(vertex->boneIds[i] >= 0) {
            vertex->weights[i] /= totalSum;
        }
    }
}

void TweenImporter::ReadJoint(u8 **file, Joint *joint) {
    joint->parent = READ_S32(*file);
    ReadString(file, joint->name);
    ReadMatrix(file, &joint->localTransform);
    ReadMatrix(file, &joint->invBindTransform);
}

void TweenImporter::ReadSample(u8 **file, AnimationSample *sample, u32 num_joints) {
    
    u32 num_animated_bones = READ_U32(*file);
    sample->localPoses = (JointPose *)MemoryManager::Get()->AllocStaticMemory(sizeof(JointPose)*num_joints, 8);

    for(u32 pose_index = 0; pose_index < num_joints; ++pose_index) {
        JointPose *pose = sample->localPoses + pose_index;
        pose->position = Vec3(0, 0, 1);
        pose->rotation = Quat(1, 0, 0, 0);
        pose->scale = Vec3(1, 1, 1);
    }
    
    bool time_stamp_initialize = false;

    for(u32 animated_bone_index = 0; animated_bone_index < num_animated_bones; ++animated_bone_index) {
        u32 bone_index = READ_U32(*file);
        f32 time_stamp = READ_F32(*file);
        ReadVec3(file, &sample->localPoses[bone_index].position);
        ReadQuat(file, &sample->localPoses[bone_index].rotation);
        ReadVec3(file, &sample->localPoses[bone_index].scale);
        
        if(time_stamp_initialize == false) {
            sample->time_stamp = time_stamp;
            time_stamp_initialize = true;
        }
    }

}

/*-----------------------------------------------------*/
/*        Model importer interface implementation      */
/*-----------------------------------------------------*/

void ModelImporter::Read(char *path) {
    MemoryManager::Get()->BeginTemporalMemory();
    
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(path);
    ReadModelFile(&model, (u8 *)file.data);
    
    MemoryManager::Get()->EndTemporalMemory();
}

void ModelImporter::ReadModelFile(Model *model, u8 *file) {
    
    u32 magic = READ_U32(file);
    ASSERT(magic == TWEEN_MAGIC);

    u32 flags = READ_U32(file);

    if(flags & TWEEN_MODEL) {
        printf("Loading model file\n");
    }

    if(flags & TWEEN_SKELETON) { 
        model->type = MODEL_TYPE_ANIMATED;
    } else {
        model->type = MODEL_TYPE_STATIC;
    }
    
    ASSERT((flags & TWEEN_SKELETON) && (flags & TWEEN_MODEL));

    model->numMeshes = READ_U32(file);
    model->meshes = (Mesh *)MemoryManager::Get()->AllocStaticMemory(sizeof(Mesh)*model->numMeshes, 8);
    
    printf("Number of meshes: %d\n", model->numMeshes);
    
    u64 sizeOfVertex = sizeof(Vertex);
    if(model->type == MODEL_TYPE_ANIMATED) {
        sizeOfVertex = sizeof(SkinVertex);
    }

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;

        mesh->numVertices = READ_U32(file);
        mesh->vertices = (Vertex *)MemoryManager::Get()->AllocStaticMemory(sizeOfVertex*mesh->numVertices, 8);

        mesh->numIndices = READ_U32(file);
        mesh->indices = (u32 *)MemoryManager::Get()->AllocStaticMemory(sizeof(u32)*mesh->numIndices, 8);
        
        ReadString(&file, mesh->material);

        printf("Num vertices: %d, indices: %d\n", mesh->numVertices, mesh->numIndices);
        printf("Material path: %s\n", mesh->material);
    }

    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex;
        for(u32 vertexIndex = 0; vertexIndex < mesh->numVertices; ++vertexIndex) {
            Vertex *vertex = nullptr;
            if(model->type == MODEL_TYPE_ANIMATED) {
                vertex = (Vertex *)(((SkinVertex *)mesh->vertices) + vertexIndex);
            } else {
                vertex = mesh->vertices + vertexIndex;
            }
            ASSERT(vertex != nullptr);
            ReadVertex(&file, vertex);
        }

        for(u32 indiceIndex = 0; indiceIndex < mesh->numIndices; ++indiceIndex) {
            mesh->indices[indiceIndex] = READ_U32(file);
        }

    }
    
    if(model->type == MODEL_TYPE_ANIMATED) {
        
        for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
            Mesh *mesh = model->meshes + meshIndex;
            for(u32 vertexIndex = 0; vertexIndex < mesh->numVertices; ++vertexIndex) {
                SkinVertex *vertex = (SkinVertex *)mesh->vertices + vertexIndex;
                // NOTE: Initiallize weights
                for(u32 i = 0; i < MAX_BONES_INFLUENCE; ++i) {
                    vertex->weights[i] = 0;
                    vertex->boneIds[i] = -1.0f;
                }
            }
        }
        
        char skeletonName[MAX_NAME_SIZE];
        ReadString(&file, skeletonName);
        u32 totalNumberOfBones = READ_U32(file);
        
        printf("Skeleton name: %s, total bones: %d\n", skeletonName, totalNumberOfBones);
        printf("Loading vertex weights for skeleton ... \n");
        
        for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
            Mesh *mesh = model->meshes + meshIndex;

            u32 numberOfBones = READ_U32(file);
            
            for(u32 boneIndex = 0; boneIndex < numberOfBones; ++boneIndex) {
                
                u32 currentBoneId = READ_U32(file);
                u32 numWeights = READ_U32(file);

                for(u32 WightsIndex = 0; WightsIndex < numWeights; ++WightsIndex) {
                    
                    u32 vertexIndex = READ_U32(file);
                    f32 weight = READ_F32(file);
                    
                    ASSERT(vertexIndex < mesh->numVertices);
                    SkinVertex *vertex = (SkinVertex *)mesh->vertices + vertexIndex;
                    AddWeightToVertex(vertex, currentBoneId, weight);
                
                }
            }
        }

        for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
            Mesh *mesh = model->meshes + meshIndex;
            for(u32 vertexIndex = 0; vertexIndex < mesh->numVertices; ++vertexIndex) {
                SkinVertex *vertex = (SkinVertex *)mesh->vertices + vertexIndex;
                NormalizeVertexWeigths(vertex);
            }
        }

        printf("All vertices ready for animation!\n");
    }
}

/*-----------------------------------------------------*/
/*    Animation importer interface implementation      */
/*-----------------------------------------------------*/

void AnimationImporter::Read(char *path) {

    MemoryManager::Get()->BeginTemporalMemory();
    
    File file = PlatformManager::Get()->ReadFileToTemporalMemory(path);
    ReadSkeletonFile(&skeleton, &animations, &numAnimations, (u8 *)file.data);
    
    MemoryManager::Get()->EndTemporalMemory();
}

void AnimationImporter::ReadSkeletonFile(Skeleton *skeleton, AnimationClip **animations, u32 *num_animations, u8 *file) {

    u32 magic = READ_U32(file);
    ASSERT(magic == TWEEN_MAGIC);

    u32 flags = READ_U32(file);

    if(flags & TWEEN_ANIMATIONS) {
        printf("Loading Animation file\n");
    }
    
    ASSERT(flags & TWEEN_ANIMATIONS);

    ReadString(&file, skeleton->name);
    skeleton->numJoints = READ_U32(file);
    skeleton->joints = (Joint *)MemoryManager::Get()->AllocStaticMemory(sizeof(Joint)*skeleton->numJoints, 8);
    printf("Loaded skeleton name: %s, number of joints: %d\n", skeleton->name, skeleton->numJoints);

    for(u32 joint_index = 0; joint_index < skeleton->numJoints; ++joint_index) {
        Joint *joint = skeleton->joints + joint_index;
        ReadJoint(&file, joint);
    }
    
    u32 animations_array_size = READ_U32(file);
    AnimationClip *animations_array = (AnimationClip *)MemoryManager::Get()->AllocStaticMemory(sizeof(AnimationClip)*animations_array_size, 8);

    printf("Number of animations: %d\n", animations_array_size);

    for(u32 animation_index = 0; animation_index < animations_array_size; ++animation_index) {
        AnimationClip *animation = animations_array + animation_index; 
        animation->skeleton = skeleton;

        ReadString(&file, animation->name);
        animation->duration = READ_F32(file);
        animation->numSamples = READ_U32(file);
        animation->samples = (AnimationSample *)MemoryManager::Get()->AllocStaticMemory(sizeof(AnimationSample)*animation->numSamples, 8);
        
        for(u32 sample_index = 0; sample_index < animation->numSamples; ++sample_index) {
            AnimationSample *sample = animation->samples + sample_index;
            ReadSample(&file, sample, skeleton->numJoints);
        }

        printf("Animation name: %s, duration: %f, keyframes: %d\n", animation->name, animation->duration, animation->numSamples);

    }
    
    *animations = animations_array;
    *num_animations = animations_array_size;

    printf("Animation complete loading perfectly!\n");
}
