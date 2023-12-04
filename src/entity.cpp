#include "entity.h"
#include "level.h"
#include "components.h"

#include <stdio.h>
#include <float.h>


// ---------------------------------------------------------
void Entity::Initialize(const char *name) {
    this->name = (char *)name;
    ClearFlags();

    next = nullptr;
    prev = nullptr;
    componentContainerList = nullptr;
}

void Entity::Initialize(const char *name, Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader, Map *map, Entity *entities, AnimationClipSet *animationSet) {
    
    Initialize(name);

    TransformComponentDesc transformDesc = {};
    transformDesc.pos = pos;
    transformDesc.rot = rot;
    transformDesc.scale = scale;
    AddComponent<TransformComponent>(&transformDesc);

    PhysicsComponentDesc physCompDesc = {};
    physCompDesc.pos = pos;
    physCompDesc.vel = Vec3();
    physCompDesc.acc = Vec3();
    physCompDesc.map = map;
    physCompDesc.entities = entities;
    AddComponent<PhysicsComponent>(&physCompDesc);

    CollisionComponentDesc collisionCompDesc = {};
    collisionCompDesc.type = COLLIDER_CYLINDER;
    collisionCompDesc.cylinder.c = pos;
    collisionCompDesc.cylinder.u = Vec3(0, 1, 0);
    collisionCompDesc.cylinder.radii = 0.3f;
    collisionCompDesc.cylinder.n = 0.75f;
    AddComponent<CollisionComponent>(&collisionCompDesc);

    GraphicsComponentDesc graphCompDesc = {};
    graphCompDesc.model = model;
    graphCompDesc.shader = shader;
    AddComponent<GraphicsComponent>(&graphCompDesc);

}

void Entity::Terminate() {

    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component = (Component *)&container->component;
        component->Terminate(this);

        container = container->next;
    }

}

void Entity::Update(Map *map, f32 dt) {

    RemoveFlag(ENTITY_COLLIDING);

    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component = (Component *)&container->component;
        component->Process(this, dt);

        container = container->next;
    }

}

void Entity::Render() {
    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component = (Component *)&container->component;
        component->Render(this);

        container = container->next;
    }
}
