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
    { {{ 0, -1,  0}, 32.0f}, {}, 0 },
    // top
    { {{ 0,  1,  0}, 32.0f}, {}, 0 },
    // left
    { {{-1,  0,  0}, 32.0f}, {}, 0 },
    // right
    { {{ 1,  0,  0}, 32.0f}, {}, 0 },
    // front
    { {{ 0,  0,  1}, 32.0f}, {}, 0 },
    // back
    { {{ 0,  0, -1}, 32.0f}, {}, 0 }
};



static void TransformCube(Vec3 *cube,  Vec3 p, Vec3 scale, f32 angle) {
    for(i32 i = 0; i < 24; ++i) {
        cube[i] = Mat4::TransformPoint(Mat4::Translate(p.x, p.y, p.z) * Mat4::RotateX(angle) * Mat4::Scale(scale), gCube[i]);
    }
}

static void DrawCube(Vec3 *cube, u32 color) {
    for(i32 faceIndex = 0; faceIndex < 6; ++faceIndex) {
        
        Vec3 *vertices = &cube[faceIndex * 4];
        
        for(i32 i = 0; i < 4; ++i) {
            Vec3 a = vertices[i];
            Vec3 b = vertices[(i + 1) % 4];

            GraphicsManager::Get()->DrawLine(a, b, color);
        }
    }
}

static void TransformEntity(MapImporter::Entity *mapEntity, Vec3 scale, Vec3 movement) {
    for(i32 i = 0; i < mapEntity->facesCount; ++i) {
        MapImporter::EntityFace *face = mapEntity->faces + i;

        f32 d = fabsf(face->plane.n.Dot(scale)) / 2;
        f32 displacement = movement.Dot(face->plane.n);
        if(displacement != 0.0f) {
            face->plane.d = (gCubeFaces[i].plane.d * d) + (displacement*32.0f);
        } else {
            face->plane.d = (gCubeFaces[i].plane.d * d);
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

// TransformComponent --------------------------------------------------
void TransformComponent::Initialize(Entity *entity, void *initData) {
    TransformComponentDesc *compDesc = (TransformComponentDesc *)initData;
    pos = compDesc->pos;
    rot = compDesc->rot;
    scale = compDesc->scale;
}

void TransformComponent::Process(Entity *entity, f32 dt) { 
    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    if(physicsComp) {
        pos = physicsComp->physics.pos;
        rot.y = physicsComp->physics.orientation;
    }
}


// GraphicsComponent ---------------------------------------------------

void GraphicsComponent::Initialize(Entity *entity, void *initData) {
    GraphicsComponentDesc *compDesc = (GraphicsComponentDesc *)initData;
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

void GraphicsComponent::Render(Entity *entity) {

    TransformComponent renderTransform = *entity->GetComponent<TransformComponent>();
    
    if(model.type == MODEL_TYPE_ANIMATED) {
        renderTransform.pos.y -= 0.75f;
        AnimationComponent *animationComp = entity->GetComponent<AnimationComponent>();
        GraphicsManager::Get()->SetAnimMatrices(animationComp->finalTransformMatrices, animationComp->numFinalTrasformMatrices);
    }

    GraphicsManager::Get()->SetWorldMatrix(renderTransform.GetWorldMatrix());
    
    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        if(mesh->indexBuffer) {
            GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, shader);
        } else {
            GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, shader);
        }
    }
}


// PhysicsComponent ---------------------------------------------------

void PhysicsComponent::Initialize(Entity *entity, void *initData) {
    PhysicsComponentDesc *compDesc = (PhysicsComponentDesc *)initData;
    physics.pos = compDesc->pos;
    physics.vel = compDesc->vel;
    physics.acc = compDesc->acc;
    physics.orientation = 0.0f;
    physics.angularVel = 0.0f;
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

    groundSegment.a = collisionComp->cylinder.c;
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
    
    Entity *testEntity = entities;
    while(testEntity) {

        if(testEntity != entity) {

            Segment groundSegment;
            Vec3 lastVelXZ = Vec3(physics.vel.x, 0.0f, physics.vel.z);
            if(lastVelXZ.LenSq() > 0.0f) {
                velXZ = lastVelXZ.Normalized();
            }
            groundSegment.a = collisionComp->cylinder.c - (velXZ * (collisionComp->cylinder.radii + 0.05f));
            groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.1), 0);

            Segment centerGroundSegment;
            centerGroundSegment.a = collisionComp->cylinder.c;
            centerGroundSegment.b = centerGroundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.1), 0);


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
            if(groundSegment.HitCollider(testCollisionComp, &t) ||
               centerGroundSegment.HitCollider(testCollisionComp, &t)) {
                entity->AddFlag(ENTITY_GROUNDED);

                // TODO: move this to other place
                // move the player realtive to the platofrm position
                MovingPlatformComponent *movPlatComp = testEntity->GetComponent<MovingPlatformComponent>();
                if(movPlatComp != nullptr) {
                    physics.pos  = physics.pos + movPlatComp->movement;
                    collisionComp->cylinder.c = physics.pos;
                }
            }
        }


        testEntity = testEntity->prev;
    }

}

void PhysicsComponent::Process(Entity *entity, f32 dt) {

    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();

    // NOTE: Apply gravity
    if(!entity->HaveFlag(ENTITY_GROUNDED))
        physics.acc += Vec3(0, -9.8 * 2.5, 0);
 
    physics.vel += physics.acc * dt;
    // NOTE: Apply drag
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
    physics.orientation += physics.angularVel * dt;

    // NOTE: Collision detection
    entity->RemoveFlag(ENTITY_GROUNDED);
    entity->RemoveFlag(ENTITY_COLLIDING); 
    ProcessMap(entity);
    ProcessEntities(entity, dt);

    lastPhysics = physics;

    physics.acc = Vec3();
    physics.angularVel = 0;

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
            TransformComponent *transformComp = entity->GetComponent<TransformComponent>();
            TransformEntity(&poly3D.entity, transformComp->scale, transformComp->pos);
        } break;
    }
}


void CollisionComponent::Render(Entity *entity) {
#if 0
    switch(type) {
        case COLLIDER_CYLINDER: {
            PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
            
            Segment groundSegment;
            
            groundSegment.a = cylinder.c - (physicsComp->velXZ * (cylinder.radii + 0.05f));
            groundSegment.b = groundSegment.a + Vec3(0, -(cylinder.n + 0.1), 0);
            GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, entity->HaveFlag(ENTITY_GROUNDED) ? 0xffff0000 : 0xff00ff00);

            groundSegment.a = cylinder.c;
            groundSegment.b = groundSegment.a + Vec3(0, -(cylinder.n + 0.1), 0);            
            GraphicsManager::Get()->DrawLine(groundSegment.a, groundSegment.b, entity->HaveFlag(ENTITY_GROUNDED) ? 0xffff0000 : 0xff00ff00);
            
            DrawCylinder(cylinder, entity->HaveFlag(ENTITY_COLLIDING) ? 0xFFFF00FF : 0xFF00FFFF);
        } break;
        case COLLIDER_CONVEXHULL: {
            DrawCube(poly3D.convexHull.points, entity->HaveFlag(ENTITY_COLLIDING) ? 0xFFFF00FF : 0xFF00FFFF);
        } break;
    }
#endif
}


// AnimationComponent ---------------------------------------------------

void AnimationComponent::Initialize(Entity *entity, void *initData) {
    AnimationComponentDesc *compDesc = (AnimationComponentDesc *)initData;
    animation.Initialize(compDesc->animations, compDesc->numAnimations);
    
    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;

    leavingAnimation = {};
    startAnimation   = {};
    endAnimation     = {};
}

void AnimationComponent::Terminate(Entity *entity) {
    animation.Terminate();
    finalTransformMatrices = nullptr;
    numFinalTrasformMatrices = 0;
}

void AnimationComponent::Process(Entity *entity, f32 dt) {
    
    if(leavingAnimation.size > 0) {
        for(u32 nameIndex = 0; nameIndex < leavingAnimation.size; ++nameIndex) {
            animation.UpdateWeightScale(leavingAnimation.names[nameIndex], 0);
        }
        ClearAnimationLeavingGroup();
    }

    if(startAnimation.size > 0) {
        for(u32 nameIndex = 0; nameIndex < startAnimation.size; ++nameIndex) {
            animation.UpdateWeightScale(startAnimation.names[nameIndex], (1.0f - tBlend));
        }
    }

    if(endAnimation.size > 0) {
        for(u32 nameIndex = 0; nameIndex < endAnimation.size; ++nameIndex) {
            animation.UpdateWeightScale(endAnimation.names[nameIndex], tBlend);
        }
    }

    animation.Update(dt, &finalTransformMatrices, &numFinalTrasformMatrices);
}

inline static void AddAnimationToGroup(AnimationGroup *group, const char *name) {
    ASSERT(group->size < MAX_ANIMATION_GROUP_SIZE);
    group->names[group->size++] = (char *)name;
}

void AnimationComponent::AddAnimationToLeavingGroup(const char *name) {
    AddAnimationToGroup(&leavingAnimation, name);
}

void AnimationComponent::AddAnimationToStartGroup(const char *name, bool loop) {
    AddAnimationToGroup(&startAnimation, name);
    animation.SetLoop(name, loop);
}

void AnimationComponent::AddAnimationToEndGroup(const char *name, bool loop) {
    AddAnimationToGroup(&endAnimation, name);
    animation.SetLoop(name, loop);
}

void AnimationComponent::SetBlendFactor(f32 tBlend) {
    this->tBlend = tBlend;
}

void AnimationComponent::ClearAnimationLeavingGroup(void) {
    leavingAnimation.size = 0;
}

void AnimationComponent::ClearAnimationStartGroup(void) {
    startAnimation.size = 0;
}

void AnimationComponent::ClearAnimationEndGroup(void) {
    endAnimation.size = 0;
}

// InputComponent --------------------------------------------------------

void InputComponent::Initialize(Entity *entity, void *initData) {
    InputComponentDesc *compDesc = (InputComponentDesc *)initData;
    input = compDesc->input;
    camera = compDesc->camera;
}

void InputComponent::Terminate(Entity *entity) {

}

void InputComponent::Process(Entity *entity, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    Vec3 worldFront = camera->GetWorldFront();
    Vec3 right = camera->right;
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

    if(entity->HaveFlag(ENTITY_GROUNDED) && (input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A))) {
        physicsComp->physics.vel += Vec3(0, 15, 0);
    }
    
    f32 acc = 40.0f;
    f32 drag = 1.0f;

    if(!entity->HaveFlag(ENTITY_GROUNDED)) {
        drag = 0.1f;
    }

    physicsComp->physics.acc += worldFront * input->state[0].leftStickY;
    physicsComp->physics.acc += right      * input->state[0].leftStickX;
    physicsComp->physics.acc *= acc * drag;


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

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    
    Input *input = PlatformManager::Get()->GetInput();

    EntityState *newState = state->Move(entity, input, *camera, dt);
    if(newState != nullptr) {
        state->Exit(entity);
        newState->Enter(entity);
        state = newState;
    }

    physicsComp->physics.orientation = camera->rot.y;
}


// MovingPlatformComponent -------------------------------------------------

void MovingPlatformComponent::Initialize(Entity *entity, void *initData) {

    MovingPlatformComponentDesc *movCompDesc = (MovingPlatformComponentDesc *)initData;
    a = movCompDesc->a;
    b = movCompDesc->b;
    dtElapsed = 0.0f;

}

void MovingPlatformComponent::Process(Entity *entity, f32 dt) {

    TransformComponent *transformComp = entity->GetComponent<TransformComponent>();
    CollisionComponent *collisionComp = entity->GetComponent<CollisionComponent>();

    Vec3 lastPos = transformComp->pos;

    f32 t = (sinf(dtElapsed) + 1.0f) * 0.5f;
    transformComp->pos = a + (b - a) * t;

    movement = transformComp->pos - lastPos;

    TransformCube(collisionComp->poly3D.convexHull.points, transformComp->pos, transformComp->scale, 0);
    TransformEntity(&collisionComp->poly3D.entity, transformComp->scale, transformComp->pos);

    dtElapsed += dt; 

}

// AIComponent -------------------------------------------------------------

void AIComponent::Initialize(Entity *entity, void *initData) {
    AIComponentDesc *compDesc = (AIComponentDesc *)initData;
    behavior = compDesc->behavior;
    timeToTarget = compDesc->timeToTarget;
    arrivalRadii = compDesc->arrivalRadii;
    active = compDesc->active;
}

void AIComponent::Process(Entity *entity, f32 dt) {
    
    PhysicsComponent *phyComp = entity->GetComponent<PhysicsComponent>();

    Steering steering = {};

    switch(behavior) {
        case STEERING_BEHAVIOR_FACE: { steering = Face(this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        case STEERING_BEHAVIOR_SEEK: { steering = Seek(this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        case STEERING_BEHAVIOR_FLEE: { steering = Flee(this, phyComp, *gBlackBoard.target, timeToTarget); } break;
    }
    
    phyComp->physics.acc += steering.linear;
    phyComp->physics.angularVel += steering.angular;

}
