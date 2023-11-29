#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "common.h"
#include "gjk_collision.h"
#include "graphics.h"
#include "animation.h"
#include "camera.h"
#include "math.h"
#include "components.h"

#include <typeinfo>

static ObjectAllocator<ComponentContainer> gComponetsAllocator;

struct Input;

enum EntityFlags {
    ENTITY_STATIC    = 1 << 0,
    ENTITY_DYNAMIC   = 1 << 1,
    
    ENTITY_GROUNDED  = 1 << 2,
    ENTITY_COLLIDING = 1 << 3,
};


struct Entity {

    void Initialize(Vec3 pos, Vec3 rot, Vec3 scale, Model model, Shader shader, Map *map, Entity *entities, AnimationClipSet *animationSet);
    void Terminate();

    void Update(Map *map, f32 dt);
    void Render();

    template <typename T>
    void AddComponent(void *initData = nullptr);
    
    template <typename T>
    void RemoveComponent();

    template <typename T>
    T *GetComponent();

    u32 flags;
    ComponentContainer *componentContainerList; 
    Entity *next;
    Entity *prev;
    
    inline void AddFlag(EntityFlags flag) { flags |= flag; }
    inline void RemoveFlag(EntityFlags flag) { flags &= ~flag; }
    inline void ClearFlags() { flags = 0; };
    inline bool HaveFlag(EntityFlags flag) { return (flags & flag) != 0; }

};


template <typename T>
void Entity::AddComponent(void *initData) {
    ComponentContainer *container = gComponetsAllocator.Alloc();
    memset(container, 0, sizeof(ComponentContainer));
    
    // Super Hack
    //T *component = (T *)&container->component;
    // T tmpComponent = {};
    // memcpy(component, &tmpComponent, sizeof(T));
    
    T *component = new(&container->component) T;

    component->Initialize(this, initData);

    if(componentContainerList == nullptr) {
        componentContainerList = container;
    } else {
        container->next = componentContainerList;
        componentContainerList->prev = container;
        componentContainerList = container; 
    }
}

template <typename T>
void Entity::RemoveComponent() {

    ComponentContainer *containerToRemove = nullptr;
    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *component =  (Component *)&container->component;

        if(typeid(*component) == typeid(T)) {
            component->Terminate(this);
            containerToRemove = container;
            break;
        }

        container = container->next;
    }

    if(containerToRemove) {

        if(containerToRemove->prev == nullptr) {
            componentContainerList = containerToRemove->next;
        } else if(containerToRemove->next ==  nullptr) {
            containerToRemove->prev->next = nullptr;
        } else {
            containerToRemove->prev->next = containerToRemove->next;
            containerToRemove->next->prev = containerToRemove->prev;
        }

        containerToRemove->next = nullptr;
        containerToRemove->prev = nullptr;
        gComponetsAllocator.Free(containerToRemove);
    }
}

template <typename T>
T *Entity::GetComponent() {

    Component *component = nullptr;
    ComponentContainer *container = componentContainerList;
    while(container) {

        Component *componentToFind = (Component *)&container->component;
        if(typeid(*componentToFind) == typeid(T)) {
            component = componentToFind;
            break;
        }

        container = container->next;
    }

    //ASSERT(component != nullptr);

    return (T *)component;
}



#endif // _ENTITY_H_
