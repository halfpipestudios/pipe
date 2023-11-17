#include "entity.h"
#include "geometry.h"
#include "level.h"

#include <stdio.h>
#include <float.h>

IdleState    Entity::idleState;
WalkingState Entity::walkingState;
JumpingState Entity::jumpingState;
FallingState Entity::fallingState;

// Idle State ...
// ---------------------------------------------------------
EntityState *IdleState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {
    
    if(!entity->HaveFlag(ENTITY_GROUNDED) && entity->physics.vel.y < 0) {
        return &entity->fallingState;
    }

    if(input->KeyIsPress(KEY_W) || input->KeyIsPress(KEY_S) || input->KeyIsPress(KEY_A) || input->KeyIsPress(KEY_D)) {
        return &entity->walkingState;
    }
    
    if((input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
        return &entity->jumpingState;
    }

    printf("idle!!\n");
    
    return nullptr;
}

void IdleState::Enter(Entity *entity) {
    if(entity->animation.IsAnimationFinish("idle")) {
        entity->animation.Play("idle", 1, true);
    } else {
        entity->animation.Continue("idle");
    }
}

void IdleState::Exit(Entity *entity) {
    entity->animation.Stop("idle");
}

// Walking State ...
// ---------------------------------------------------------
EntityState *WalkingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {
    
    Vec2 vel2d = Vec2(entity->physics.vel.x, entity->physics.vel.z);
    entity->animation.UpdateWeight("walking", CLAMP(vel2d.Len()*0.25f, 0, 1));


    Vec3 worldFront = camera.GetWorldFront();
    Vec3 right = camera.right;
    if(input->KeyIsPress(KEY_W)) {
        entity->physics.acc += worldFront;
    }
    if(input->KeyIsPress(KEY_S)) {
        entity->physics.acc -= worldFront;
    }
    if(input->KeyIsPress(KEY_A)) {
        entity->physics.acc -= right;
    }
    if(input->KeyIsPress(KEY_D)) {
        entity->physics.acc += right;
    }

    entity->physics.acc += worldFront * input->state[0].leftStickY;
    entity->physics.acc += right      * input->state[0].leftStickX;
    entity->physics.acc *= 40.0f;

    if((input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
        return &entity->jumpingState;
    }

    if(entity->physics.vel.Len() < 0.01f) {
        return &entity->idleState;
    }

    if(!entity->HaveFlag(ENTITY_GROUNDED) && entity->physics.vel.y < 0) {
        return &entity->fallingState;
    }
    
    printf("walking!!\n");

    return nullptr;
}

void WalkingState::Enter(Entity *entity) {
    if(entity->animation.IsAnimationFinish("idle")) {
        entity->animation.Play("idle", 1, true);
    } else {
        entity->animation.Continue("idle");
    }
    entity->animation.Play("walking", 0, true);
}

void WalkingState::Exit(Entity *entity) {
    entity->animation.Pause("idle");
    entity->animation.Stop("walking");
}

// Jumping State ...
// ---------------------------------------------------------

EntityState *JumpingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {
    
    entity->physics.acc = entity->physics.acc * 0.1f;

    if(!entity->HaveFlag(ENTITY_GROUNDED) && entity->physics.vel.y < 0) {
        return &entity->fallingState;
    }

    if(entity->HaveFlag(ENTITY_GROUNDED)) {
        return &entity->idleState;
    }
    
    printf("jumping!!\n");

    return nullptr;
}

void JumpingState::Enter(Entity *entity) {
    entity->physics.vel += Vec3(0, 15, 0);
    entity->animation.PlaySmooth("jump", 0.08f);
}

void JumpingState::Exit(Entity *entity) {
}

// Falling State ...
// ---------------------------------------------------------
EntityState *FallingState::Move(Entity *entity, Input *input, Camera camera, f32 dt) {

    entity->physics.acc = entity->physics.acc * 0.1f;
    
    if(entity->HaveFlag(ENTITY_GROUNDED)) {

        Vec2 vel2d = Vec2(entity->physics.vel.x, entity->physics.vel.z);
        if(vel2d.Len()) {
            return &entity->walkingState;
        }

        return &entity->idleState;
    }

    printf("falling!!\n");
    
    return nullptr;
}

void FallingState::Enter(Entity *entity) {
    entity->animation.Play("idle", 1, true);
}

void FallingState::Exit(Entity *entity) {
    entity->animation.Stop("idle");
}

// ---------------------------------------------------------

static void DrawCylinder(Cylinder cylinder, u32 color) {
    Vec3 vertices[40] = {};

    // Top face
    f32 increment = (2.0f * PI) / 20;
    f32 angle = 0.0f;
    for(i32 i = 0; i < 20; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c + cylinder.u * cylinder.n;
        angle += increment;
    }

    // Bottom face
    angle = 0.0f;
    for(i32 i = 20; i < 40; ++i) {
        vertices[i] = Vec3(sinf(angle), 0, cosf(angle)) * cylinder.radii + cylinder.c - cylinder.u * cylinder.n;
        angle += increment;
    }

    // Rendering code
    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[i];
        Vec3 b = vertices[(i + 1) % 20];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[20 + i];
        Vec3 b = vertices[20 + (i + 1) % 20];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

    for(i32 i = 0; i < 20; ++i) {
        Vec3 a = vertices[i];
        Vec3 b = vertices[20 + i];

        GraphicsManager::Get()->DrawLine(a, b, color);
    }

}


void Entity::Initialize(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations) {
    
    ClearFlags();

    transform.pos = pos;
    transform.rot = rot;
    transform.scale = scale;

    physics.pos = pos;
    physics.vel = Vec3();
    physics.acc = Vec3();
    lastPhysics = physics;

    this->model = model;
    animation.Initialize(animations, numAnimations);
    
    state = &idleState;

    collider.c = physics.pos;
    collider.u = Vec3(0, 1, 0);
    collider.radii = 0.3f;
    collider.n = 0.75f;

    velXZ = Vec3();
    jumpTimer = 0.0f;
    jumpStarted = false;

    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;

    next = nullptr;
}

void Entity::Terminate() {

    animation.Terminate();

    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;

    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        GraphicsManager::Get()->DestroyTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DestroyVertexBuffer(mesh->vertexBuffer);
        GraphicsManager::Get()->DestroyIndexBuffer(mesh->indexBuffer);
    }
}

void Entity::Update(Map *map, f32 dt) {

    RemoveFlag(ENTITY_COLLIDING);

    if(!HaveFlag(ENTITY_GROUNDED))
        physics.acc += Vec3(0, -9.8 * 2.5, 0);

    physics.vel += physics.acc * dt;

    if(HaveFlag(ENTITY_GROUNDED)) {
        f32 dammping = powf(0.001f, dt);
        physics.vel = physics.vel * dammping;
    }
    else {
        f32 dammping = powf(0.5f, dt);
        physics.vel = physics.vel * dammping;
    }

    physics.pos += physics.vel * dt;

    collider.c = physics.pos;

    Segment playerSegment;
    playerSegment.a =  lastPhysics.pos;
    playerSegment.b = physics.pos;

    f32 tMin = FLT_MAX; 
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *entity = &map->entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(entity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }
    if(tMin >= 0.0f && tMin <= 1.0f) {
        physics.pos = lastPhysics.pos + (physics.pos - lastPhysics.pos) * (tMin*0.8);
        collider.c = physics.pos;
    }

    GJK gjk;
    for(i32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &collider);
        if(collisionData.hasCollision) {
            AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            physics.pos += normal * penetration; 
            physics.vel -= physics.vel.Dot(normal)*normal;
            collider.c = physics.pos;
        }
    }

    Segment groundSegment;
    
    Vec3 lastVelXZ = Vec3(physics.vel.x, 0.0f, physics.vel.z);
    
    if(lastVelXZ.LenSq() > 0.0f) {
        velXZ = lastVelXZ.Normalized();
    }

    groundSegment.a = collider.c - (velXZ * (collider.radii + 0.05f));
    //groundSegment.b = groundSegment.a + Vec3(0, -(collider.n + 0.05), 0);
    groundSegment.b = groundSegment.a + Vec3(0, -(collider.n + 0.001), 0);
    RemoveFlag(ENTITY_GROUNDED);
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *entity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(entity, &t)) {
            AddFlag(ENTITY_GROUNDED);
        }
    }

    transform.pos = physics.pos;

    lastPhysics = physics;

    physics.acc = Vec3();

    animation.Update(dt, &finalTransformMatrices, &numFinalTrasformMatrices);
}

void Entity::Render(Shader shader) {
    Transform renderTransform = transform;
    renderTransform.pos.y -= 0.75f;
    GraphicsManager::Get()->SetWorldMatrix(renderTransform.GetWorldMatrix());
    GraphicsManager::Get()->SetAnimMatrices(finalTransformMatrices, numFinalTrasformMatrices);
    
    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, shader);
    }

    Segment groundSegment;
    groundSegment.a = collider.c - (velXZ * (collider.radii + 0.05f));
    //groundSegment.b = groundSegment.a + Vec3(0, -(collider.n + 0.05), 0);
    groundSegment.b = groundSegment.a + Vec3(0, -(collider.n + 0.001), 0);
    GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, HaveFlag(ENTITY_GROUNDED) ? 0xffff0000 : 0xff00ff00);
}

void Entity::Move(Input *input, Camera camera, f32 dt) {
    
    EntityState *newState = state->Move(this, input, camera, dt);
    if(newState != nullptr) {
        state->Exit(this);
        newState->Enter(this);
        state = newState;
    }
    
    transform.rot.y = camera.rot.y;

}
