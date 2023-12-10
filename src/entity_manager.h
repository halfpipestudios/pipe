#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

#include "component_storage.h"
#include "entity.h"

#define ENTITY_ARRAY_MAX_SIZE 100

struct EntityManager {

    void Initialize() {
        entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
        componentsStorage.Initialize();
    }

    template <typename ComponentType>
    void AddComponentType() {
        componentsStorage.AddComponentType<ComponentType>();
    }

    template <typename ComponentType>
    Array<ComponentType>& GetComponents() {
        return componentsStorage.GetComponents<ComponentType>();
    }

    Array<Entity_>& GetEntities() {
        return entities;
    }

    Entity_ *AddEntity() {
        Entity_ *entity = entities.Push({});
        entity->componentsPtrs.Initialize(COMPONENTS_ARRAY_MAP_SIZE);
        return entity;
    }

    template <typename ComponentType>
    ComponentType *AddComponent(Entity_ *entity) {
        Array<ComponentType>& componentsArray = GetComponents<ComponentType>();
        ComponentType *component = componentsArray.Push({});
        component->entity = entity;
        entity->componentsPtrs.Add(ComponentType::GetID(), (CMPBase *)component);
        return component;
    }

    Array<Entity_> entities;
    ComponentStorage componentsStorage;
};

#endif // _ENTITY_MANAGER_H_
