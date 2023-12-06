#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

#include "component_storage.h"

#define ENTITY_ARRAY_MAX_SIZE 100

struct EntityManager {
    template <typename ComponentType>
    void AddComponentType() {
        componentsStorage.AddComponentType<ComponentType>();
    }

    template <typename ComponentType>
    StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE>* GetComponents() {
        return componentsStorage.GetComponents<ComponentType>();
    }

    StaticArray<Entity, ENTITY_ARRAY_MAX_SIZE> *GetEntities() {
        return &entities;
    }

    Entity *AddEntity() {
        return entities.Push({});
    }

    template <typename ComponentType>
    ComponentType *AddComponent(Entity *entity) {
        StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE>* componentsArray = GetComponents<ComponentType>();
        ComponentType *component = componentsArray->Push({});
        entity->componentsPtrs.Add(ComponentType::GetID(), (ComponentBase *)component);
        return component;
    }

    StaticArray<Entity, ENTITY_ARRAY_MAX_SIZE> entities;
    ComponentStorage componentsStorage;
};

#endif // _ENTITY_MANAGER_H_
