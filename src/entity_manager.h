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
        entity->componentsIds.Initialize(COMPONENTS_ARRAY_MAP_SIZE);
        return entity;
    }

    void DeleteEntity(Entity_ *entity) {

        u64 index = (u64)entity;
        u64 start = (u64)entities.data;
        u64 end   = (u64)(entities.data + entities.size);

        ASSERT(index >= start);
        ASSERT(index < end);
        
        u32 entityIndex = (index - start) / sizeof(Entity_);

        // TODO: first delete all entity components
        printf("##########################################\n");
        for(i32 i = 0; i < entity->componentsIds.size; ++i) {
            u32 cmpID = entity->componentsIds[i]; 
            CMPBase *cmp = entity->componentsPtrs.Get(cmpID);

            ComponentArrayBase *componets = componentsStorage.GetComponentsByID(cmp->id);
             
            componets->DestroyComponent((void *)cmp);
            printf("Deleting Compoents ID: %d\n", cmp->id);


        }
        printf("##########################################\n");


        entities[entityIndex] = entities[entities.size - 1];
        entities[entities.size - 1] = {};
        
        Entity_ *moveEntity = &entities[entityIndex];
        for(i32 i = 0; i < moveEntity->componentsIds.size; ++i) {
            CMPBase *cmp = moveEntity->componentsPtrs.Get(moveEntity->componentsIds[i]);
            i32 before = 0;
            cmp->entity = moveEntity;
            i32 after = 0;
        }

        --entities.size;

    }

    template <typename ComponentType>
    ComponentType *AddComponent(Entity_ *entity) {
        Array<ComponentType>& componentsArray = GetComponents<ComponentType>();
        ComponentType *component = componentsArray.Push({});
        component->entity = entity;
        entity->componentsPtrs.Add(ComponentType::GetID(), (CMPBase *)component);
        entity->componentsIds.Push(ComponentType::GetID());
        return component;
    }

    Array<Entity_> entities;
    ComponentStorage componentsStorage;
};

#endif // _ENTITY_MANAGER_H_
