#include "components.h"
#include "geometry.h"
#include "entity.h"
#include "camera.h"
#include "level.h"

#include <float.h>
#include <stdio.h>

static Vec3 gCube[] = {

    // bottom
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    // top
    Vec3(-0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f, -0.5f),
    Vec3(-0.5, 0.5f, -0.5f),
    // left
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3(-0.5,  0.5f,  0.5f),
    // right
    Vec3(0.5, -0.5f,  0.5f),
    Vec3(0.5, -0.5f, -0.5f),
    Vec3(0.5,  0.5f, -0.5f),
    Vec3(0.5,  0.5f,  0.5f),
    // front
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3( 0.5,  0.5f, -0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    // back
    Vec3(-0.5, -0.5f, 0.5f),
    Vec3(-0.5,  0.5f, 0.5f),
    Vec3( 0.5,  0.5f, 0.5f),
    Vec3( 0.5, -0.5f, 0.5f)

};

static MapImporter::EntityFace gCubeFaces[] = {
    // bottom
    { {{ 0, -1,  0}, 1.0f*32.0f}, {}, 0 },
    // top
    { {{ 0,  1,  0}, 0.5f*32.0f}, {}, 0 },
    // left
    { {{-1,  0,  0}, 1.0f*32.0f}, {}, 0 },
    // right
    { {{ 1,  0,  0}, 1.0f*32.0f}, {}, 0 },
    // front
    { {{ 0,  0,  1}, 1.0f*32.0f}, {}, 0 },
    // back
    { {{ 0,  0, -1}, 1.0f*32.0f}, {}, 0 }
};



void TransformCube(Vec3 *cube,  Vec3 p, Vec3 scale, f32 angle) {
    for(i32 i = 0; i < 24; ++i) {
        cube[i] = Mat4::TransformPoint(Mat4::Translate(p.x, p.y, p.z) * Mat4::RotateX(angle) * Mat4::Scale(scale), gCube[i]);
    }
}

void TransformEntity(MapImporter::Entity *mapEntity, Vec3 movement) {
    for(i32 i = 0; i < mapEntity->facesCount; ++i) {
        MapImporter::EntityFace *face = mapEntity->faces + i;

        f32 displacement = movement.Dot(face->plane.n);
        if(displacement != 0.0f) {
            face->plane.d = gCubeFaces[i].plane.d + (displacement*32.0f);
        }

    }
}

void DrawCube(Vec3 *cube, u32 color) {
    for(i32 faceIndex = 0; faceIndex < 6; ++faceIndex) {
        
        Vec3 *vertices = &cube[faceIndex * 4];
        
        for(i32 i = 0; i < 4; ++i) {
            Vec3 a = vertices[i];
            Vec3 b = vertices[(i + 1) % 4];

            GraphicsManager::Get()->DrawLine(a, b, color);
        }
    }
}

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

// GraphicsComponent ---------------------------------------------------

void GraphicsComponent::Initialize(Entity *entity, void *initData) {
    GraphicsComponentDesc *compDesc = (GraphicsComponentDesc *)initData;
    transform.pos = compDesc->pos;
    transform.rot = compDesc->rot;
    transform.scale = compDesc->scale;
    model = compDesc->model;
    shader = compDesc->shader;
}

void GraphicsComponent::Terminate(Entity *entity) {
    // TODO: the component should not release the meshes of the model
    // this model should be use by multiples entities and graphics components
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

void GraphicsComponent::Render(Entity *entity) {

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
    entities = compDesc->entities;
    lastPhysics = physics;
}

void PhysicsComponent::Terminate(Entity *entity) {

}

void PhysicsComponent::ProcessMap(Entity *entity) {

    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();

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
        collisionComp->cylinder.c = physics.pos;
    }

    GJK gjk;
    for(i32 i = 0; i < map->covexHulls.count; ++i) {
        ConvexHull *hull = &map->covexHulls.data[i];
        CollisionData collisionData = gjk.Intersect(hull, &collisionComp->cylinder);
        if(collisionData.hasCollision) {
            entity->AddFlag(ENTITY_COLLIDING);
            f32 penetration = collisionData.penetration;
            Vec3 normal = collisionData.normal;
            physics.pos += normal * penetration; 
            physics.vel -= physics.vel.Dot(normal)*normal;
            collisionComp->cylinder.c = physics.pos;
        }
    }

    Segment groundSegment;
    
    Vec3 lastVelXZ = Vec3(physics.vel.x, 0.0f, physics.vel.z);
    
    if(lastVelXZ.LenSq() > 0.0f) {
        velXZ = lastVelXZ.Normalized();
    }

    groundSegment.a = collisionComp->cylinder.c - (velXZ * (collisionComp->cylinder.radii + 0.05f));
    groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.001), 0);
    for(i32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

}

void PhysicsComponent::ProcessEntities(Entity *entity, f32 dt) {

    GJK gjk;
    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();

    Segment groundSegment;
    Vec3 lastVelXZ = Vec3(physics.vel.x, 0.0f, physics.vel.z);
    if(lastVelXZ.LenSq() > 0.0f) {
        velXZ = lastVelXZ.Normalized();
    }
    groundSegment.a = collisionComp->cylinder.c - (velXZ * (collisionComp->cylinder.radii + 0.05f));
    groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.001), 0);
    
    Entity *testEntity = entities;
    while(testEntity) {

        if(testEntity != entity) {

            CollisionComponent *testCollisionComp = testEntity->GetComponent<CollisionComponent>();
     
            CollisionData collisionData = {};
            switch(testCollisionComp->type) {
                case COLLIDER_CYLINDER: {
                    collisionData = gjk.Intersect(&testCollisionComp->cylinder, &collisionComp->cylinder);
                } break;
                case COLLIDER_CONVEXHULL: {
                    collisionData = gjk.Intersect(&testCollisionComp->poly3D.convexHull, &collisionComp->cylinder);
                }
            }

            if(collisionData.hasCollision) {
                entity->AddFlag(ENTITY_COLLIDING);
                f32 penetration = collisionData.penetration;
                Vec3 normal = collisionData.normal;
                physics.pos += normal * penetration; 
                physics.vel -= physics.vel.Dot(normal)*normal;
                collisionComp->cylinder.c = physics.pos;
            }

            f32 t = -1.0f;
            if(groundSegment.HitCollider(testCollisionComp, &t)) {
                entity->AddFlag(ENTITY_GROUNDED);

                // TODO: move this to other place
                // move the player realtive to the platofrm position
                MovingPlatformComponent *movPlatComp = testEntity->GetComponent<MovingPlatformComponent>();
                if(movPlatComp != nullptr) {

                    f32 t = (sinf(movPlatComp->dtElapsed) + 1.0f) * 0.5f;
                    Vec3 newPlatforPos = movPlatComp->a + (movPlatComp->b - movPlatComp->a) * t;

                    Vec3 offset = newPlatforPos - movPlatComp->pos;
                     
                    physics.pos  = physics.pos + offset;
                }

            }
        }

        testEntity = testEntity->prev;
    }

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

    collisionComp->cylinder.c = physics.pos;

    entity->RemoveFlag(ENTITY_GROUNDED);
    entity->RemoveFlag(ENTITY_COLLIDING);
    
    ProcessMap(entity);
    ProcessEntities(entity, dt);

    lastPhysics = physics;

    physics.acc = Vec3();

}

// CollisionComponent ---------------------------------------------------

void CollisionComponent::Initialize(Entity *entity, void *initData) {

    CollisionComponentDesc *collisionCompDesc = (CollisionComponentDesc *)initData;
    type = collisionCompDesc->type;
    switch(type) {
        case COLLIDER_CYLINDER: {
            cylinder.c = collisionCompDesc->cylinder.c;
            cylinder.u = collisionCompDesc->cylinder.u;
            cylinder.radii = collisionCompDesc->cylinder.radii;
            cylinder.n = collisionCompDesc->cylinder.n; 
        } break;
        case COLLIDER_CONVEXHULL: {
            poly3D.convexHull.points = collisionCompDesc->poly3D.convexHull.points;
            poly3D.convexHull.count = collisionCompDesc->poly3D.convexHull.count;
            poly3D.entity.faces = collisionCompDesc->poly3D.entity.faces;
            poly3D.entity.facesCount = collisionCompDesc->poly3D.entity.facesCount;
        } break;
    }


}

void CollisionComponent::Process(Entity *entity, f32 dt) {
    switch(type) {
        case COLLIDER_CONVEXHULL: {
            MovingPlatformComponent *movComp = entity->GetComponent<MovingPlatformComponent>();
            TransformEntity(&poly3D.entity, movComp->pos);
        } break;
    }
}


void CollisionComponent::Render(Entity *entity) {

    switch(type) {
        case COLLIDER_CYLINDER: {
            PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
            Segment groundSegment;
            groundSegment.a = cylinder.c - (physicsComp->velXZ * (cylinder.radii + 0.05f));
            groundSegment.b = groundSegment.a + Vec3(0, -(cylinder.n + 0.001), 0);
            GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, entity->HaveFlag(ENTITY_GROUNDED) ? 0xffff0000 : 0xff00ff00);
            DrawCylinder(cylinder, entity->HaveFlag(ENTITY_COLLIDING) ? 0xFFFF00FF : 0xFF00FFFF);
        } break;
        case COLLIDER_CONVEXHULL: {
            DrawCube(poly3D.convexHull.points, entity->HaveFlag(ENTITY_COLLIDING) ? 0xFFFF00FF : 0xFF00FFFF);
        } break;
    }

}


// AnimationComponent ---------------------------------------------------

void AnimationComponent::Initialize(Entity *entity, void *initData) {
    AnimationComponentDesc *compDesc = (AnimationComponentDesc *)initData;
    animation.Initialize(compDesc->animations, compDesc->numAnimations);
    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;

    animation.Play("idle", 1, true);
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


// MovingPlatformComponent -------------------------------------------------

void MovingPlatformComponent::Initialize(Entity *entity, void *initData) {

    MovingPlatformComponentDesc *movCompDesc = (MovingPlatformComponentDesc *)initData;
    pos = movCompDesc->pos;
    a = movCompDesc->a;
    b = movCompDesc->b;
    dtElapsed = 0.0f;

}

void MovingPlatformComponent::Process(Entity *entity, f32 dt) {

    f32 t = (sinf(dtElapsed) + 1.0f) * 0.5f;
    pos = a + (b - a) * t;

    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();
    TransformCube(collisionComp->poly3D.convexHull.points, pos, Vec3(2, 1, 2), 0);

    dtElapsed += dt; 

}

