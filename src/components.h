#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include "common.h"
#include "math.h"
#include "graphics.h"
#include "gjk_collision.h"
#include "animation.h"


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

struct Entity;
struct Map;
struct Camera;

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


// ----------------------------------------------------------------------------------

struct Component {
    virtual void Initialize(Entity *entity, void *initData) {}
    virtual void Terminate(Entity *entity) {}
    virtual void Process(Entity *entity, f32 dt) {}
    virtual void Render(Entity *entity, Shader shader) {}
};

struct GraphicsComponentDesc {
    Vec3 pos;
    Vec3 rot;
    Vec3 scale;
    Model model;
};

struct GraphicsComponent : public Component {
    Transform transform;
    Model model;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;
    void Render(Entity *entity, Shader shader) override;
};

struct PhysicsComponentDesc {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;
    Map *map;
};

struct PhysicsComponent : public Component {
    Physics physics;
    Physics lastPhysics;
    Vec3 velXZ;
    Map *map;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;
};

struct CollisionComponentDesc {
    Vec3 c;
    Vec3 u;
    f32 radii;
    f32 n;
};

struct CollisionComponent : public Component {
    Cylinder collider;

    void Initialize(Entity *entity, void *initData) override;
    void Render(Entity *entity, Shader shader) override;
};

struct AnimationComponentDesc {
    AnimationClip *animations;
    u32 numAnimations;
};

struct AnimationComponent : public Component {
    AnimationSet animation;
    Mat4 *finalTransformMatrices;
    u32 numFinalTrasformMatrices;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;
};

struct InputComponent : public Component {
    void Update(Entity *entity);
};

struct StateMachineComponentDesc {
    Camera *camera;
};

struct StateMachineComponent : public Component {

    EntityState *state;
    Camera *camera;

    friend struct IdleState;
    friend struct WalkingState;
    friend struct JumpingState;
    friend struct FallingState;
    
    IdleState idleState;
    WalkingState walkingState;
    JumpingState jumpingState;
    FallingState fallingState;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;

};

// ----------------------------------------------------------------------------------

union ComponentUnion {
    GraphicsComponent graphic;
    PhysicsComponent physics;
    CollisionComponent collision;
    AnimationComponent animation;
    InputComponent input;
    StateMachineComponent stateMachine;
};

struct ComponentContainer {
    ComponentUnion component;
    ComponentContainer *next;
    ComponentContainer *prev;
};

#endif // _COMPONENT_H_
