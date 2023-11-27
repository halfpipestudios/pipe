#include "components.h"
#include "geometry.h"
#include "entity.h"
#include "camera.h"
#include "level.h"

#include <float.h>

// GraphicsComponent ---------------------------------------------------

void GraphicsComponent::Initialize(Entity *entity, void *initData) {
    GraphicsComponentDesc *compDesc = (GraphicsComponentDesc *)initData;
    transform.pos = compDesc->pos;
    transform.rot = compDesc->rot;
    transform.scale = compDesc->scale;
    model = compDesc->model;
}

void GraphicsComponent::Terminate(Entity *entity) {
    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        if(mesh->texture) GraphicsManager::Get()->DestroyTextureBuffer(mesh->texture);
        if(mesh->vertexBuffer) GraphicsManager::Get()->DestroyVertexBuffer(mesh->vertexBuffer);
        if(mesh->indexBuffer) GraphicsManager::Get()->DestroyIndexBuffer(mesh->indexBuffer);
    }
}


void GraphicsComponent::Process(Entity *entity, f32 dt) { 
    // TODO: the graphics component should not deepend on the physics component
    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    transform.pos = physicsComp->physics.pos;
}

void GraphicsComponent::Render(Entity *entity, Shader shader) {

    AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();

    Transform renderTransform = transform;
    renderTransform.pos.y -= 0.75f;
    GraphicsManager::Get()->SetWorldMatrix(renderTransform.GetWorldMatrix());
    GraphicsManager::Get()->SetAnimMatrices(animationComp->finalTransformMatrices, animationComp->numFinalTrasformMatrices);
    
    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, shader);
    }
}


// PhysicsComponent ---------------------------------------------------

void PhysicsComponent::Initialize(Entity *entity, void *initData) {
    PhysicsComponentDesc *compDesc = (PhysicsComponentDesc *)initData;
    physics.pos = compDesc->pos;
    physics.vel = compDesc->vel;
    physics.acc = compDesc->acc;
    velXZ = Vec3(physics.vel.x, 0, physics.vel.z);
    map = compDesc->map;
    lastPhysics = physics;
}

void PhysicsComponent::Terminate(Entity *entity) {

}

void PhysicsComponent::Process(Entity *entity, f32 dt) {

    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();

    if(!entity->HaveFlag(ENTITY_GROUNDED))
        physics.acc += Vec3(0, -9.8 * 2.5, 0);

    physics.vel += physics.acc * dt;

    if(entity->HaveFlag(ENTITY_GROUNDED)) {
        f32 dammping = powf(0.001f, dt);
        physics.vel = physics.vel * dammping;
    }
    else {
        f32 dammping = powf(0.5f, dt);
        physics.vel = physics.vel * dammping;
    }

    physics.pos += physics.vel * dt;

    collisionComp->collider.c = physics.pos;

    Segment playerSegment;
    playerSegment.a =  lastPhysics.pos;
    playerSegment.b = physics.pos;

    f32 tMin = FLT_MAX; 
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(mapEntity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }
    if(tMin >= 0.0f && tMin <= 1.0f) {
        physics.pos = lastPhysics.pos + (physics.pos - lastPhysics.pos) * (tMin*0.8);
        collisionComp->collider.c = physics.pos;
    }

    GJK gjk;
    for(i32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &collisionComp->collider);
        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            physics.pos += normal * penetration; 
            physics.vel -= physics.vel.Dot(normal)*normal;
            collisionComp->collider.c = physics.pos;
        }
    }

    Segment groundSegment;
    
    Vec3 lastVelXZ = Vec3(physics.vel.x, 0.0f, physics.vel.z);
    
    if(lastVelXZ.LenSq() > 0.0f) {
        velXZ = lastVelXZ.Normalized();
    }

    groundSegment.a = collisionComp->collider.c - (velXZ * (collisionComp->collider.radii + 0.05f));
    groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->collider.n + 0.001), 0);
    entity->RemoveFlag(ENTITY_GROUNDED);
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

    lastPhysics = physics;

    physics.acc = Vec3();

}

// CollisionComponent ---------------------------------------------------

void CollisionComponent::Initialize(Entity *entity, void *initData) {

    CollisionComponentDesc *collisionCompDesc = (CollisionComponentDesc *)initData;

    collider.c = collisionCompDesc->c;
    collider.u = collisionCompDesc->u;
    collider.radii = collisionCompDesc->radii;
    collider.n = collisionCompDesc->n; 
}


void CollisionComponent::Render(Entity *entity, Shader shader) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();

    Segment groundSegment;
    groundSegment.a = collider.c - (physicsComp->velXZ * (collider.radii + 0.05f));
    groundSegment.b = groundSegment.a + Vec3(0, -(collider.n + 0.001), 0);
    GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, entity->HaveFlag(ENTITY_GROUNDED) ? 0xffff0000 : 0xff00ff00);
}


// AnimationComponent ---------------------------------------------------

void AnimationComponent::Initialize(Entity *entity, void *initData) {
    AnimationComponentDesc *compDesc = (AnimationComponentDesc *)initData;
    animation.Initialize(compDesc->animations, compDesc->numAnimations);
    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;
}

void AnimationComponent::Terminate(Entity *entity) {
    animation.Terminate();
    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;
}

void AnimationComponent::Process(Entity *entity, f32 dt) {

    animation.Update(dt, &finalTransformMatrices, &numFinalTrasformMatrices);

}


// InputComponent --------------------------------------------------------

void InputComponent::Update(Entity *entity) {


}

// StateMachineComponent -------------------------------------------------

void StateMachineComponent::Initialize(Entity *entity, void *initData) {

    StateMachineComponentDesc *compDesc = (StateMachineComponentDesc *)initData;

    state = (EntityState *)&idleState;
    camera = compDesc->camera;
}

void StateMachineComponent::Terminate(Entity *entity) {

}

void StateMachineComponent::Process(Entity *entity, f32 dt) {

    GraphicsComponent *graphicsComp = entity->GetComponent<GraphicsComponent>();
    
    Input *input = PlatformManager::Get()->GetInput();

    EntityState *newState = state->Move(entity, input, *camera, dt);
    if(newState != nullptr) {
        state->Exit(entity);
        newState->Enter(entity);
        state = newState;
    }

    graphicsComp->transform.rot.y = camera->rot.y;
}

