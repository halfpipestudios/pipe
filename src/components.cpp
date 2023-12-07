#include "components.h"
#include "geometry.h"
#include "entity.h"
#include "camera.h"
#include "level.h"
#include "behavior_tree.h"

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
    f32 increment = (2.0f * (f32)PI) / 20;
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
        rot.y = physicsComp->physics.orientation - (f32)PI*0.5f;
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
        PlayerAnimationComponent *animationComp = entity->GetComponent<PlayerAnimationComponent>();
        if(animationComp != nullptr) {
            GraphicsManager::Get()->SetAnimMatrices(animationComp->finalTransformMatrix, animationComp->numFinalTransformMatrix);
        }
        else {

            // TODO: refactor this. this is just to not animate the entities with
            // no AnimationComponent
            Mat4 identity[100];
            for(i32 i = 0; i < 100; ++i) {
                identity[i] = Mat4();
            }
            GraphicsManager::Get()->SetAnimMatrices(identity, 100);
        }
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
    physics.orientation = 0;
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
#if 0
    f32 tMin = FLT_MAX; 
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(playerSegment.HitEntity(mapEntity, &t)) {
           if(t < tMin) {
                tMin = t;
            }
        }
    }
    if(tMin >= 0.0f && tMin <= 1.0f) {
        physics.pos = lastPhysics.pos + (physics.pos - lastPhysics.pos) * (tMin*0.8f);
        collisionComp->cylinder.c = physics.pos;
    }
#endif

    GJK gjk;
    for(u32 i = 0; i < map->covexHulls.count; ++i) {
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
    groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.01f), 0);
    for(u32 i = 0; i < map->entities.count; ++i) {
        MapImporter::Entity *mapEntity = &map->entities.data[i];
        f32 t = -1.0f;
        if(groundSegment.HitEntity(mapEntity, &t)) {
            entity->AddFlag(ENTITY_GROUNDED);
        }
    }

    groundSegment.a = collisionComp->cylinder.c;
    groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.01f), 0);
    for(u32 i = 0; i < map->entities.count; ++i) {
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
            groundSegment.b = groundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.1f), 0);

            Segment centerGroundSegment;
            centerGroundSegment.a = collisionComp->cylinder.c;
            centerGroundSegment.b = centerGroundSegment.a + Vec3(0, -(collisionComp->cylinder.n + 0.1f), 0);


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


    // TODO: the player orientatin should not depend on the velocity direction,
    // the velocity direction should depend on the angular velocity. 
    // TODO: add and angular velocity to the player
    Vec3 dir = { physicsComp->physics.vel.x, 0.0f, physicsComp->physics.vel.z };
    dir.Normalize();
    f32 orientation = atan2f(dir.z, dir.x);
    physicsComp->physics.orientation = orientation;


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
    bhTree = compDesc->bhTree;
}

void AIComponent::Process(Entity *entity, f32 dt) {

    PhysicsComponent *phyComp = entity->GetComponent<PhysicsComponent>();

    if(bhTree) {
        BehaviorNodeContex contx;
        contx.entity = entity;
        contx.phyComp = phyComp;
        contx.aiComp = this;
        bhTree->run(&contx);
        return;
    }
    
    Steering steering = {};

    switch(behavior) {
        case STEERING_BEHAVIOR_FACE:   { steering = Face(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        case STEERING_BEHAVIOR_SEEK:   { steering = Seek(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        case STEERING_BEHAVIOR_FLEE:   { steering = Flee(  this, phyComp, *gBlackBoard.target, timeToTarget); } break;
        case STEERING_BEHAVIOR_ARRIVE: { steering = Arrive(this, phyComp, *gBlackBoard.target, timeToTarget); } break;
    }
    
    phyComp->physics.acc += steering.linear;
    phyComp->physics.angularVel += steering.angular;

}

// Player Animaton State ------------------------------------
// ----------------------------------------------------------

void PlayerAnimationState::CalculateCurrentAnimationFrame(Entity *entity, f32 dt) {
    Skeleton *skeleton = anim->animationSet->skeleton;
    ASSERT(skeleton != nullptr);
    
    anim->finalTransformMatrix = (Mat4 *)MemoryManager::Get()->AllocFrameMemory(sizeof(Mat4)*anim->numFinalTransformMatrix, 8);
    
    MemoryManager::Get()->BeginTemporalMemory();
    
    JointPose *pose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose) * skeleton->numJoints, 8); 
    
    if(anim->transition.InProgress()) {
        anim->transition.SampleJointPose(pose, entity, dt);
    } else {
        SampleJointPose(pose, entity, dt);
    }
    
    CalculateFinalTransformMatrices(pose, anim->finalTransformMatrix, skeleton);
    
    MemoryManager::Get()->EndTemporalMemory();

}

// Idle Animation state -------------------------------

void PlayerAnimationIdleState::Initialize(PlayerAnimationComponent *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    idleAnimation.Initialize(set->FindAnimationClipByName("idle"), -1, true);
}

void PlayerAnimationIdleState::SampleJointPose(JointPose *pose, Entity *entity, f32 dt) {
    idleAnimation.SampleNextAnimationPose(pose, dt);
}

PlayerAnimationState *PlayerAnimationIdleState::Update(Entity *entity, Input *input, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);
    
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;
    if(!anim->transition.InProgress()) {
        if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < e) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y > e) {
            anim->transition.Start(this, &anim->jump, 0.2f);
        }
    }
    
    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;

}

void PlayerAnimationIdleState::Enter(Entity *entity) {
}

void PlayerAnimationIdleState::Exit(Entity *entity) {
}

// Walk Animation state -------------------------------

void PlayerAnimationWalkState::Initialize(PlayerAnimationComponent *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    idleAnimation.Initialize(set->FindAnimationClipByName("idle"), -1, true);
    walkAnimation.Initialize(set->FindAnimationClipByName("walking"), -1, true);
}

void PlayerAnimationWalkState::SampleJointPose(JointPose *pose, Entity *entity, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    
    f32 t = CLAMP(vel2d.Len() * 0.25f, 0, 1);
    u32 numJoints = anim->animationSet->skeleton->numJoints;

    MemoryManager::Get()->BeginTemporalMemory();
    
    JointPose *idlePose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    JointPose *walkPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    
    idleAnimation.SampleNextAnimationPose(idlePose, dt);
    walkAnimation.SampleNextAnimationPose(walkPose, dt);
    JointPoseMixSamples(pose, idlePose, walkPose, numJoints, t);
    
    MemoryManager::Get()->EndTemporalMemory();
}

PlayerAnimationState *PlayerAnimationWalkState::Update(Entity *entity, Input *input, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);

    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;
    if(!anim->transition.InProgress()) {
        if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y > e) {
            anim->transition.Start(this, &anim->jump, 0.2f);
        } else if(physicsComp->physics.vel.Len() < 0.01f) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        } else if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < e) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;

}

void PlayerAnimationWalkState::Enter(Entity *entity) {
}

void PlayerAnimationWalkState::Exit(Entity *entity) {
}

// Jump Animation state -------------------------------

void PlayerAnimationJumpState::Initialize(PlayerAnimationComponent *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    jumpAnimation.Initialize(set->FindAnimationClipByName("jump"), -1, false);
}

void PlayerAnimationJumpState::SampleJointPose(JointPose *pose, Entity *entity, f32 dt) {
    
    if(jumpAnimation.time > 0.42f) {
        jumpAnimation.SampleAnimationPose(pose, 0.42f);
    } else {
        jumpAnimation.SampleNextAnimationPose(pose, dt);
    }
}

PlayerAnimationState *PlayerAnimationJumpState::Update(Entity *entity, Input *input, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);

    f32 e = 0.01f;

    if(!anim->transition.InProgress()) {
        if(!entity->HaveFlag(ENTITY_GROUNDED) && physicsComp->physics.vel.y < e) {
            anim->transition.Start(this, &anim->fall, 0.2f);
        } else if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(physicsComp->physics.vel.Len() < e) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);
    
    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;
}

void PlayerAnimationJumpState::Enter(Entity *entity) {
}

void PlayerAnimationJumpState::Exit(Entity *entity) {
    jumpAnimation.time = 0;
}

// Fall Animation state -------------------------------

void PlayerAnimationFallState::Initialize(PlayerAnimationComponent *component) {
    anim = component;
    AnimationClipSet *set = component->animationSet;
    fallAnimation.Initialize(set->FindAnimationClipByName("jump"), -1, true);
}

void PlayerAnimationFallState::SampleJointPose(JointPose *pose, Entity *entity, f32 dt) {
    fallAnimation.SampleAnimationPose(pose, 0.42f);
}

PlayerAnimationState *PlayerAnimationFallState::Update(Entity *entity, Input *input, f32 dt) {

    PhysicsComponent *physicsComp = entity->GetComponent<PhysicsComponent>();
    ASSERT(physicsComp != nullptr);
    
    Vec2 vel2d = Vec2(physicsComp->physics.vel.x, physicsComp->physics.vel.z);
    f32 e = 0.01f;

    if(!anim->transition.InProgress()) {
        if(entity->HaveFlag(ENTITY_GROUNDED) && vel2d.Len() > e) {
            anim->transition.Start(this, &anim->walk, 0.2f);
        } else if(physicsComp->physics.vel.Len() < e) {
            anim->transition.Start(this, &anim->idle, 0.2f);
        }
    }

    CalculateCurrentAnimationFrame(entity, dt);

    if(anim->transition.Finished()) {
        return anim->transition.GetNextState();
    }

    return nullptr;
}

void PlayerAnimationFallState::Enter(Entity *entity) {
}

void PlayerAnimationFallState::Exit(Entity *entity) {
}

// Player Animaton Component -------------------------------
// ---------------------------------------------------------
    
void PlayerAnimationComponent::Initialize(Entity *entity, void *initData) {
    
    PlayerAnimationComponentDesc *desc = (PlayerAnimationComponentDesc *)initData;
    animationSet = desc->animationSet;

    numFinalTransformMatrix = animationSet->skeleton->numJoints;

    idle.Initialize(this);
    walk.Initialize(this);
    jump.Initialize(this);
    fall.Initialize(this);

    state = &idle;
}

void PlayerAnimationComponent::Terminate(Entity *entity) {
}


static void AdjustAngle(f32& angle) {
    while(angle >  (f32)PI) angle -= (f32)(2*PI);
    while(angle < -(f32)PI) angle += (f32)(2*PI);
}

void PlayerAnimationComponent::Process(Entity *entity, f32 dt) {
    Input *input = PlatformManager::Get()->GetInput();
    
    PlayerAnimationState *newState = state->Update(entity, input, dt);
    if(newState != nullptr) {
        state->Exit(entity);
        newState->Enter(entity);
        state = newState;
    }
}

// Player Animaton Transition -------------------------------
// ----------------------------------------------------------

void PlayerAnimationTransition::Start(PlayerAnimationState *src, PlayerAnimationState *des, f32 duration) {
    ASSERT(inTransition == false);
    this->inTransition = true;
    this->duration = duration;
    this->time = 0;
    this->src = src;
    this->des = des;
}

void PlayerAnimationTransition::SampleJointPose(JointPose *pose, Entity *entity, f32 dt) {
    
    ASSERT(inTransition == true);
    ASSERT(duration > 0);

    f32 t = time / duration;

    ASSERT(src->anim->animationSet->skeleton->numJoints == des->anim->animationSet->skeleton->numJoints)
    u32 numJoints = src->anim->animationSet->skeleton->numJoints;
    
    MemoryManager::Get()->BeginTemporalMemory();

    JointPose *srcPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    JointPose *desPose = (JointPose *)MemoryManager::Get()->AllocTemporalMemory(sizeof(JointPose)*numJoints, 8);
    
    src->SampleJointPose(srcPose, entity, dt);
    des->SampleJointPose(desPose, entity, dt);
    JointPoseMixSamples(pose, srcPose, desPose, numJoints, t);

    MemoryManager::Get()->EndTemporalMemory();

    time += dt;

    if(time > duration) inTransition = false;

}

bool PlayerAnimationTransition::InProgress() {
    return inTransition == true && time <= duration;
}

bool PlayerAnimationTransition::Finished() {
    return inTransition == false && time > duration;
}

PlayerAnimationState *PlayerAnimationTransition::GetNextState() {
    ASSERT(Finished());
    ASSERT(des != nullptr);
    time = 0;
    return des;
}
