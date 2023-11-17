#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "common.h"
#include "gjk_collision.h"
#include "graphics.h"
#include "animation.h"
#include "camera.h"
#include "math.h"

struct Map;
struct Input;

struct Transform {
    Vec3 pos;
    Vec3 rot;
    Vec3 scale;
    inline Mat4 GetWorldMatrix() { return Mat4::Translate(pos) * Mat4::Rotate(rot) * Mat4::Scale(scale); };
};

struct Physics {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;
};

enum EntityFlags {
    ENTITY_STATIC    = 1 << 0,
    ENTITY_DYNAMIC   = 1 << 1,
    
    ENTITY_GROUNDED  = 1 << 2,
    ENTITY_COLLIDING = 1 << 3,
};

struct Entity;

struct EntityState {
    virtual EntityState *Move(Entity *entity, Input *input, Camera camera, f32 dt) = 0;
    virtual void Enter(Entity *entity) = 0;
    virtual void Exit(Entity *entity) = 0;
};

struct IdleState : public EntityState {
    EntityState *Move(Entity *entity, Input *input, Camera camera, f32 dt) override;
    void Enter(Entity *entity) override;
    void Exit(Entity *entity) override;
};

struct WalkingState : public EntityState {
    EntityState *Move(Entity *entity, Input *input, Camera camera, f32 dt) override;
    void Enter(Entity *entity) override;
    void Exit(Entity *entity) override;
};

struct JumpingState : public EntityState {
    EntityState *Move(Entity *entity, Input *input, Camera camera, f32 dt) override;
    void Enter(Entity *entity) override;
    void Exit(Entity *entity) override;
};

struct FallingState : public EntityState {
    EntityState *Move(Entity *entity, Input *input, Camera camera, f32 dt) override;
    void Enter(Entity *entity) override;
    void Exit(Entity *entity) override;
};

struct Entity {

    void Initialize(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations);
    void Terminate();

    void Update(Map *map, f32 dt);
    void Render(Shader shader);

    void Move(Input *input, Camera camera, f32 dt);
    
    Entity *next;

    Transform transform;
    Physics physics;
    Physics lastPhysics;
    Cylinder collider;

    Vec3 velXZ;
    f32 jumpTimer;
    bool jumpStarted;

    AnimationSet animation;
private:
    
    u32 flags;
    EntityState *state;

    Model model;
    
    Mat4 *finalTransformMatrices;
    u32 numFinalTrasformMatrices;

    inline void AddFlag(EntityFlags flag) { flags |= flag; }
    inline void RemoveFlag(EntityFlags flag) { flags &= ~flag; }
    inline void ClearFlags() { flags = 0; };
    inline bool HaveFlag(EntityFlags flag) { return (flags & flag) != 0; }

    friend struct IdleState;
    friend struct WalkingState;
    friend struct JumpingState;
    friend struct FallingState;
    
    static IdleState idleState;
    static WalkingState walkingState;
    static JumpingState jumpingState;
    static FallingState fallingState;
};




#endif // _ENTITY_H_
