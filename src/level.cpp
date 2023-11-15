#include "level.h"
#include "geometry.h"

#include <float.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static TextureBuffer LoadTextureFromPath(char *path) {
    static char diffuse_material_path_cstr[4096];
    sprintf(diffuse_material_path_cstr, "%s%s", "./data/textures/", path); 
    
    stbi_set_flip_vertically_on_load(true);
    i32 w, h, n;
    u32 *bitmap = (u32 *)stbi_load(diffuse_material_path_cstr, &w, &h, &n, 4);
    
    Texture texture = {bitmap, w, h};
    TextureBuffer textureBuffer = GraphicsManager::Get()->CreateTextureBuffer(&texture, 1);
    
    stbi_image_free(bitmap);

    printf("Texture: %s loaded\n", diffuse_material_path_cstr);

    return textureBuffer;
}

static void LoadModelToGpu(Model *model) {
    ASSERT(model->type == MODEL_TYPE_ANIMATED);
    for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
        Mesh *mesh = model->meshes + meshIndex; 
        mesh->texture = LoadTextureFromPath(mesh->material);
        mesh->vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer((SkinVertex *)mesh->vertices, mesh->numVertices);
        mesh->indexBuffer = GraphicsManager::Get()->CreateIndexBuffer(mesh->indices, mesh->numIndices);
    }
}

Entity *Level::AddEntity(Vec3 pos, Vec3 rot, Vec3 scale, Model model, AnimationClip *animations, u32 numAnimations) {
    Entity *entity = entitiesAllocator.Alloc();
    entity->Initialize(pos, rot, scale, model, animations, numAnimations);
    entity->next = entities;
    entities = entity;
    return entity;
}

void Level::Initialize(char *mapFilePath) {
    entities = nullptr;

    // NOTE Load Map ------------------------------------------------------------------------------------------
    MapImporter mapImporter;
    mapImporter.LoadMapFromFile(mapFilePath);
    MapImporter::VertexArray mapVertices = mapImporter.GetVertices();
    MapImporter::TextureArray mapTextures = mapImporter.GetTextures();
    
    map.covexHulls = mapImporter.GetConvexHulls();
    map.entities = mapImporter.GetEntities();
    map.vertexBuffer = GraphicsManager::Get()->CreateVertexBuffer(mapVertices.data, mapVertices.count);
    map.texture = GraphicsManager::Get()->CreateTextureBuffer(mapTextures.data, mapTextures.count);
    map.scale = 1.0f/32.0f;
    
    // NOTE Load entities ------------------------------------------------------------------------------------
    ModelImporter modelImporter;
    AnimationImporter animationImporter;
    
    modelImporter.Read("./data/models/orc.twm");
    animationImporter.Read("./data/models/orc.twa");
    LoadModelToGpu(&modelImporter.model);
    orc = AddEntity(Vec3(6, 2, 0), Vec3(), Vec3(1, 1, 1),
                    modelImporter.model, animationImporter.animations, 
                    animationImporter.numAnimations);

    modelImporter.Read("./data/models/hero.twm");
    animationImporter.Read("./data/models/hero.twa");
    LoadModelToGpu(&modelImporter.model);
    hero = AddEntity(Vec3(0, 30, 0), Vec3(), Vec3(0.8f, 0.8f, 0.8f),
                     modelImporter.model, animationImporter.animations, 
                     animationImporter.numAnimations);


    camera.Initialize();
}

void Level::Terminate() {
    GraphicsManager::Get()->DestroyTextureBuffer(map.texture);
    GraphicsManager::Get()->DestroyVertexBuffer(map.vertexBuffer);

    Entity *entity = entities;
    while(entity != nullptr) {
        
        Entity *toFree = entity;
        entity = entity->next; 

        toFree->Terminate();
        entitiesAllocator.Free(toFree);
    }

}

void Level::Update(f32 dt) {

    Input *input = PlatformManager::Get()->GetInput();

    hero->Move(input, camera);
    Entity *entity = entities;
    while(entity != nullptr) {
        entity->Update(&map, dt);
        entity = entity->next;
    }

    camera.target = hero->collider.c;
    camera.ProcessMovement(input, &map, dt);
    camera.SetViewMatrix();
}

void Level::Render(Shader statShader, Shader animShader) {
    
    GraphicsManager::Get()->SetWorldMatrix(Mat4::Scale(map.scale, map.scale, map.scale));
    GraphicsManager::Get()->BindTextureBuffer(map.texture);
    GraphicsManager::Get()->DrawVertexBuffer(map.vertexBuffer, statShader);

    Entity *entity = entities;
    while(entity != nullptr) {
        entity->Render(animShader);
        entity = entity->next;
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
    animation.Play("idle", 1, true);
    animation.Play("walking", 0, true);
    
    SetColliderPos();
    collider.u = Vec3(0, 1, 0);
    collider.radii = 0.3f;
    collider.n = 0.75f;

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
        physics.acc += Vec3(0, -9.8 * 5, 0);

    physics.vel += physics.acc * dt;

    f32 dammping = powf(0.001f, dt);
    physics.vel = physics.vel * dammping;

    physics.pos += physics.vel * dt;

    Vec2 vel2d = Vec2(physics.vel.x, physics.vel.z);
    animation.UpdateWeight("walking", CLAMP(vel2d.Len()*0.25f, 0, 1));
    animation.Update(dt, &finalTransformMatrices, &numFinalTrasformMatrices);

    SetColliderPos();
 
    /*
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
        SetColliderPos();
    }
    */

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
            SetColliderPos();
        }
    }

    Segment groundSegment;
    groundSegment.a = collider.c;
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
}

void Entity::Render(Shader shader) {

    GraphicsManager::Get()->SetWorldMatrix(transform.GetWorldMatrix());
    GraphicsManager::Get()->SetAnimMatrices(finalTransformMatrices, numFinalTrasformMatrices);
    
    for(u32 meshIndex = 0; meshIndex < model.numMeshes; ++meshIndex) {
        Mesh *mesh = model.meshes + meshIndex;
        GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
        GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, shader);
    }

   // DrawCylinder(collider, HaveFlag(ENTITY_COLLIDING) ? 0xffff0000 : 0xff00ff00);
}

void Entity::Move(Input *input, Camera camera) {

    Vec3 worldFront = camera.GetWorldFront();
    Vec3 right = camera.right;
    if(input->KeyIsPress(KEY_W)) {
        physics.acc += worldFront;
    }
    if(input->KeyIsPress(KEY_S)) {
        physics.acc -= worldFront;
    }
    if(input->KeyIsPress(KEY_A)) {
        physics.acc -= right;
    }
    if(input->KeyIsPress(KEY_D)) {
        physics.acc += right;
    }
    if((input->KeyJustPress(KEY_SPACE) || input->JoystickJustPress(JOYSTICK_BUTTON_A)) && HaveFlag(ENTITY_GROUNDED)) {
        physics.vel += Vec3(0, 40, 0);
    }

    physics.acc += worldFront * input->state[0].leftStickY;
    physics.acc += right      * input->state[0].leftStickX;

    //if(physics.acc.LenSq() > 0.0f) physics.acc.Normalize();
    physics.acc *= 40.0f;

    transform.rot.y = camera.rot.y;
}
