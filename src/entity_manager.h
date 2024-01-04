#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

#include "component_storage.h"
#include "entity.h"

#define ENTITY_ARRAY_MAX_SIZE 120

struct EntityManager {

    void Initialize() {
        entities.Initialize(ENTITY_ARRAY_MAX_SIZE);
        componentsStorage.Initialize();
    }

    void Terminate() {
        // destroy all entities and component stored on the entity manager
        for(i32 i = 0; i < entities.data.size; ++i) {
            Entity_ *entity = &entities.data[i];
            for(i32 i = 0; i < entity->componentsIds.size; ++i) {
                u32 cmpID = entity->componentsIds[i]; 
                ComponentSlotmapBase* slotmap = componentsStorage.GetComponentsSlotmapById(cmpID);
                SlotmapKey componentKey = entity->componentsKeys.Get(cmpID);
                slotmap->DestroyComponent(componentKey);
            }            
        }
    
    }

    template <typename ComponentType>
    void AddComponentType() {
        componentsStorage.AddComponentType<ComponentType>();
    }

    template <typename ComponentType>
    Array<ComponentType>& GetComponents() {
        return componentsStorage.GetComponents<ComponentType>();
    }

    template <typename ComponentType>
    ComponentType *GetComponent(SlotmapKey entityKey) {
        Entity_ *entity = &entities.Get(entityKey);
        SlotmapKey componentKey = entity->componentsKeys.Get(ComponentType::GetID());

        if(componentKey.gen == INVALID_KEY) {
            return nullptr;
        }

        Slotmap<ComponentType>& slotmap = componentsStorage.GetComponentsSlotmap<ComponentType>();
        ComponentType *component = &slotmap.Get(componentKey);
        return component;
    }

#if 0
    // TODO: see if this is realy necesary
    template <typename ComponentType>
    ComponentType *GetComponentFromEntityPtr(Entity_ *entity) {
        SlotmapKey componentKey = entity->componentsKeys.Get(ComponentType::GetID());

        if(componentKey.gen == INVALID_KEY) {
            return nullptr;
        }

        Slotmap<ComponentType>& slotmap = componentsStorage.GetComponentsSlotmap<ComponentType>();
        ComponentType *component = &slotmap.Get(componentKey);
        return component;
    }
#endif

    Array<Entity_>& GetEntities() {
        return entities.data;
    }

    Entity_ *GetEntity(SlotmapKey entityKey) {
        if(entityKey.gen == INVALID_KEY) {
            return nullptr;
        }
        return &entities.Get(entityKey);
    }

    SlotmapKey AddEntity() {
        SlotmapKey entityKey = entities.Add({});
        Entity_ *entity = &entities.Get(entityKey);
        entity->componentsKeys.Initialize(COMPONENTS_TYPE_MAX_SIZE);
        entity->componentsIds.Initialize(COMPONENTS_TYPE_MAX_SIZE);
        return entityKey;
    }


    void DeleteEntity(SlotmapKey entityKey) {

        Entity_ *entity = &entities.Get(entityKey);

        for(i32 i = 0; i < entity->componentsIds.size; ++i) {
            u32 cmpID = entity->componentsIds[i]; 
            ComponentSlotmapBase* slotmap = componentsStorage.GetComponentsSlotmapById(cmpID);
            SlotmapKey componentKey = entity->componentsKeys.Get(cmpID);
            slotmap->DestroyComponent(componentKey);
        }

        entities.Remove(entityKey);
    }


    template <typename ComponentType>
    ComponentType *AddComponent(SlotmapKey entityKey) {
        // get a pointer to the entity
        Entity_ *entity = &entities.Get(entityKey);
        // Get the slotmap of the current component type
        Slotmap<ComponentType>& slotmap = componentsStorage.GetComponentsSlotmap<ComponentType>();
        // add a new component to the slotmap
        SlotmapKey componentKey = slotmap.Add(ComponentType{});
        // get the component to initilize its entity
        ComponentType *component = &slotmap.Get(componentKey);
        component->entityKey = entityKey;
        // the the key and component type id to the entity too
        entity->componentsKeys.Add(ComponentType::GetID(), componentKey);
        entity->componentsIds.Push(ComponentType::GetID());
        // return a pointer to the component 
        return component;
    }

private:

    Slotmap<Entity_> entities;
    ComponentStorage componentsStorage;
};

#endif // _ENTITY_MANAGER_H_
