#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include "common.h"
#include "math.h"
#include "graphics.h"
#include "gjk_collision.h"
#include "animation.h"
#include "steering_behaviors.h"

#include "map_importer.h"

struct Physics {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;

    f32 orientation;
    f32 angularVel;
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
    virtual void Render(Entity *entity) {}
};

struct TransformComponentDesc {
    Vec3 pos;
    Vec3 rot;
    Vec3 scale;
};

struct TransformComponent : public Component {
    Vec3 pos;
    Vec3 rot;
    Vec3 scale;
    inline Mat4 GetWorldMatrix() { return Mat4::Translate(pos) * Mat4::Rotate(rot) * Mat4::Scale(scale); };

    void Initialize(Entity *entity, void *initData) override;
    void Process(Entity *entity, f32 dt) override;
};

struct GraphicsComponentDesc {
    Model model;
    Shader shader;
};

struct GraphicsComponent : public Component {
    // Transform transform;
    Model model;
    Shader shader;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Render(Entity *entity) override;
};

struct PhysicsComponentDesc {
    Vec3 pos;
    Vec3 vel;
    Vec3 acc;
    Map *map;
    Entity *entities;
};

struct PhysicsComponent : public Component {
    Physics physics;
    Physics lastPhysics;
    Vec3 velXZ;
    Map *map;
    Entity *entities;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;

private:
    void ProcessMap(Entity *entity);
    void ProcessEntities(Entity *entity, f32 dt);
};

enum ColliderType {
    COLLIDER_CYLINDER,
    COLLIDER_CONVEXHULL
};

struct CollisionComponentDesc {
    ColliderType type;
    union {
        Cylinder cylinder;
        struct Poly3D {
            ConvexHull convexHull;
            MapImporter::Entity entity;
        } poly3D;
    };
};

struct CollisionComponent : public Component {
    ColliderType type;
    union {
        Cylinder cylinder;
        struct Poly3D {
            ConvexHull convexHull;
            // TODO: not use a MapImporter::Entity here ...
            MapImporter::Entity entity;
        } poly3D;
    };

    CollisionComponent() {}
    void Initialize(Entity *entity, void *initData) override;
    void Process(Entity *entity, f32 dt) override;
    void Render(Entity *entity) override;
};

struct AnimationComponentDesc {
    AnimationClip *animations;
    u32 numAnimations;
};

#define MAX_ANIMATION_GROUP_SIZE 8
struct AnimationGroup {
    char *names[MAX_ANIMATION_GROUP_SIZE];
    u32 size;
};

struct AnimationComponent : public Component {
    
    AnimationSet animation;
    Mat4 *finalTransformMatrices;
    u32 numFinalTrasformMatrices;
    
    f32 tBlend;

    AnimationGroup leavingAnimation;
    
    AnimationGroup startAnimation;
    AnimationGroup endAnimation;

    void AddAnimationToLeavingGroup(const char *name);
    void AddAnimationToStartGroup(const char *name, bool loop = true);
    void AddAnimationToEndGroup(const char *name, bool loop = true);
    
    void ClearAnimationLeavingGroup(void);
    void ClearAnimationStartGroup(void);
    void ClearAnimationEndGroup(void);
    
    void SetBlendFactor(f32 tBlend);

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;
};

struct InputComponentDesc {
    Input *input;
    Camera *camera;
};

struct InputComponent : public Component {
    
    Input *input;
    Camera *camera;

    void Initialize(Entity *entity, void *initData) override;
    void Terminate(Entity *entity) override;
    void Process(Entity *entity, f32 dt) override;
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

struct MovingPlatformComponentDesc {
    Vec3 a, b;
};

struct MovingPlatformComponent : public Component {
    Vec3 a, b;
    Vec3 movement;
    f32 dtElapsed;

    void Initialize(Entity *entity, void *initData) override;
    void Process(Entity *entity, f32 dt) override;
};

struct AIComponentDesc {
    SteeringBehavior behavior;
    f32 timeToTarget;
    f32 arrivalRadii;
    f32 active;
};

struct AIComponent : public Component {

    SteeringBehavior behavior;
    f32 timeToTarget;
    f32 arrivalRadii;
    f32 active;
    
    void Initialize(Entity *entity, void *initData) override;
    void Process(Entity *entity, f32 dt) override;

};

// ----------------------------------------------------------------------------------

union ComponentUnion {
    TransformComponent transform;
    GraphicsComponent graphic;
    PhysicsComponent physics;
    CollisionComponent collision;
    AnimationComponent animation;
    InputComponent input;
    StateMachineComponent stateMachine;
    MovingPlatformComponent movPlatform;
    AIComponent ai;
};

struct ComponentContainer {
    ComponentUnion component;
    ComponentContainer *next;
    ComponentContainer *prev;
};

#endif // _COMPONENT_H_
