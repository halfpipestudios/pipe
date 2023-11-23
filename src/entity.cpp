#include "entity.h"
#include "level.h"
#include "components.h"

#include <stdio.h>
#include <float.h>

// Idle State ...
// ---------------------------------------------------------
EntityState *IdleState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    StateMachineComponent *stateMachineComp = entity->GetComponent<StateMachineComponent>();
    ASSERT(stateMachineComp != nullptr);
    
    if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < 0) {
        return &stateMachineComp->fallingState;
    }

    if(input->KeyIsPress(KEY_W) || input->KeyIsPress(KEY_S) || input->KeyIsPress(KEY_A) || input->KeyIsPress(KEY_D)) {
        return &stateMachineComp->walkingState;
    }
    
    if((input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
        return &stateMachineComp->jumpingState;
    }

    //printf("idle!!\n");
    
    return nullptr;
}

void IdleState::Enter(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    if(animationComp->animation.IsAnimationFinish("idle")) {
        animationComp->animation.Play("idle", 1, true);
    } else {
        animationComp->animation.Continue("idle");
    }
}

void IdleState::Exit(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    animationComp->animation.Stop("idle");
}

// Walking State ...
// ---------------------------------------------------------
EntityState *WalkingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    StateMachineComponent *stateMachineComp = entity->GetComponent<StateMachineComponent>();
    ASSERT(stateMachineComp != nullptr);
    
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    animationComp->animation.UpdateWeight("walking", CLAMP(vel2d.Len()*0.25f, 0, 1));


    Vec3 worldFront = camera.GetWorldFront();
    Vec3 right = camera.right;
    if(input->KeyIsPress(KEY_W)) {
        physicsComp->physics.acc += worldFront;
    }
    if(input->KeyIsPress(KEY_S)) {
        physicsComp->physics.acc -= worldFront;
    }
    if(input->KeyIsPress(KEY_A)) {
        physicsComp->physics.acc -= right;
    }
    if(input->KeyIsPress(KEY_D)) {
        physicsComp->physics.acc += right;
    }

    physicsComp->physics.acc += worldFront * input->state[0].leftStickY;
    physicsComp->physics.acc += right      * input->state[0].leftStickX;
    physicsComp->physics.acc *= 40.0f;

    if((input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
        return &stateMachineComp->jumpingState;
    }

    if(physicsComp->physics.vel.Len() < 0.01f) {
        return &stateMachineComp->idleState;
    }

    if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < 0) {
        return &stateMachineComp->fallingState;
    }
    
    //printf("walking!!\n");

    return nullptr;
}

void WalkingState::Enter(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    if(animationComp->animation.IsAnimationFinish("idle")) {
        animationComp->animation.Play("idle", 1, true);
    } else {
        animationComp->animation.Continue("idle");
    }
    animationComp->animation.Play("walking", 0, true);
}

void WalkingState::Exit(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    animationComp->animation.Pause("idle");
    animationComp->animation.Stop("walking");
}

// Jumping State ...
// ---------------------------------------------------------

EntityState *JumpingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    StateMachineComponent *stateMachineComp = entity->GetComponent<StateMachineComponent>();
    ASSERT(stateMachineComp != nullptr);
    
    physicsComp->physics.acc = physicsComp->physics.acc * 0.1f;

    if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < 0) {
        return &stateMachineComp->fallingState;
    }

    if(entity->HaveFlag(ENTITY_GROUNDED)) {
        return &stateMachineComp->idleState;
    }
    
    //printf("jumping!!\n");

    return nullptr;
}

void JumpingState::Enter(Entity *entity) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    physicsComp->physics.vel += Vec3(0, 15, 0);
    animationComp->animation.PlaySmooth("jump", 0.08f);
}

void JumpingState::Exit(Entity *entity) {
}

// Falling State ...
// ---------------------------------------------------------
EntityState *FallingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    StateMachineComponent *stateMachineComp = entity->GetComponent<StateMachineComponent>();
    ASSERT(stateMachineComp != nullptr);

    physicsComp->physics.acc = physicsComp->physics.acc * 0.1f;
    
    if(entity->HaveFlag(ENTITY_GROUNDED)) {

        Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
        if(vel2d.Len()) {
            return &stateMachineComp->walkingState;
        }

        return &stateMachineComp->idleState;
    }

    //printf("falling!!\n");
    
    return nullptr;
}

void FallingState::Enter(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    animationComp->animation.Play("idle", 1, true);
}

void FallingState::Exit(Entity *entity) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
    ASSERT(animationComp != nullptr);

    animationComp->animation.Stop("idle");
}

// ---------------------------------------------------------

void Entity::Initialize(Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader, AnimationClip *animations, u32 numAnimations, Map *map, Entity *entities) {
    
    ClearFlags();

    next = nullptr;
    prev = nullptr;
    componentContainerList = nullptr;

    GraphicsComponentDesc graphCompDesc = {};
    graphCompDesc.pos = pos;
    graphCompDesc.rot = rot;
    graphCompDesc.scale = scale;
    graphCompDesc.model = model;
    graphCompDesc.shader = shader;
    AddComponent<GraphicsComponent>(&graphCompDesc);

    PhysicsComponentDesc physCompDesc = {};
    physCompDesc.pos = pos;
    physCompDesc.vel = Vec3();
    physCompDesc.acc = Vec3();
    physCompDesc.map = map;
    physCompDesc.entities = entities;
    AddComponent<PhysicsComponent>(&physCompDesc);

    AnimationComponentDesc animCompDesc = {};
    animCompDesc.animations = animations;
    animCompDesc.numAnimations = numAnimations;
    AddComponent<AnimationComponent>(&animCompDesc);

    CollisionComponentDesc collisionCompDesc = {};
    collisionCompDesc.type = COLLIDER_CYLINDER;
    collisionCompDesc.cylinder.c = pos;
    collisionCompDesc.cylinder.u = Vec3(0, 1, 0);
    collisionCompDesc.cylinder.radii = 0.3f;
    collisionCompDesc.cylinder.n = 0.75f;
    AddComponent<CollisionComponent>(&collisionCompDesc);

}

void Entity::Terminate() {

    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component =  (Component *)&container->component;
        component->Terminate(this);

        container = container->next;
    }

}

void Entity::Update(Map *map, f32 dt) {

    RemoveFlag(ENTITY_COLLIDING);

    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component =  (Component *)&container->component;
        component->Process(this, dt);

        container = container->next;
    }

    Input *input = PlatformManager::Get()->GetInput();
    if(input->KeyJustPress(KEY_K)) {
        RemoveComponent<AnimationComponent>();
    }
}

void Entity::Render() {
    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component =  (Component *)&container->component;
        component->Render(this);

        container = container->next;
    }
}
