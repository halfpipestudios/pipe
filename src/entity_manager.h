#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

#include "component_storage.h"
#include "entity.h"

#define ENTITY_ARRAY_MAX_SIZE 100

struct EntityManager {
    template <typename ComponentType>
    void AddComponentType() {
        componentsStorage.AddComponentType<ComponentType>();
    }

    template <typename ComponentType>
    StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE>& GetComponents() {
        return componentsStorage.GetComponents<ComponentType>();
    }

    StaticArray<Entity_, ENTITY_ARRAY_MAX_SIZE>& GetEntities() {
        return entities;
    }

    Entity_ *AddEntity() {
        return entities.Push({});
    }

    template <typename ComponentType>
    ComponentType *AddComponent(Entity_ *entity) {
        StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE>& componentsArray = GetComponents<ComponentType>();
        ComponentType *component = componentsArray.Push({});
        component->entity = entity;
        entity->componentsPtrs.Add(ComponentType::GetID(), (CMPBase *)component);
        return component;
    }

    StaticArray<Entity_, ENTITY_ARRAY_MAX_SIZE> entities;
    ComponentStorage componentsStorage;
};

#endif // _ENTITY_MANAGER_H_
