#ifndef _COMPONENT_STORAGE_H_
#define _COMPONENT_STORAGE_H_

#include "data_structures.inl"
#include "cmp/base_cmp.h"

#define COMPONENTS_ARRAY_MAP_SIZE 64
#define COMPONENTS_ARRAY_MAX_SIZE 256

struct ComponentArrayBase {
    virtual void DestroyComponent(void *ptr) = 0;
};

template <typename ComponentType>
struct ComponentArray : ComponentArrayBase {
    Array<ComponentType> components;

    void DestroyComponent(void *ptr) override {
       
        u64 index = (u64)ptr;
        u64 start = (u64)components.data;
        u64 end   = (u64)(components.data + components.size);

        ASSERT(index >= start);
        ASSERT(index < end);
        
        u32 i = (index - start) / sizeof(ComponentType);
        ComponentType *lastCmp = &components[components.size - 1];

        //components[i] = components[components.size - 1];
        memcpy(components.data + i, components.data + components.size - 1, sizeof(ComponentType));
        
        u32 cmpID = ComponentType::GetID();        
        CMPBase **lastEntityCmp = lastCmp->entity->componentsPtrs.GetPtr(cmpID);
        *lastEntityCmp = components.data + i;

        memset(components.data + components.size - 1, 0, sizeof(ComponentType));

        --components.size;
    }
};

struct ComponentStorage {

    void Initialize() {
        componentsArraysMap.Initialize(COMPONENTS_ARRAY_MAP_SIZE);
    }

    template <typename ComponentType>
    Array<ComponentType>& GetComponents() {
        i32 id = ComponentType::GetID(); 
        ComponentArray<ComponentType> *componentArray = (ComponentArray<ComponentType> *)componentsArraysMap.Get(id);
        return componentArray->components;
    }

    ComponentArrayBase *GetComponentsByID(i32 id) {
        ComponentArrayBase *componentArray = componentsArraysMap.Get(id);
        return componentArray;
    }

    template <typename ComponentType>
    void AddComponentType() {
        i32 id = ComponentType::GetID(); 
        void *buffer = MemoryManager::Get()->AllocStaticMemory(sizeof(ComponentArray<ComponentType>), 1);
        ComponentArray<ComponentType> *componentArray = new(buffer) ComponentArray<ComponentType>;
        componentArray->components.Initialize(COMPONENTS_ARRAY_MAX_SIZE);
        componentsArraysMap.Add(id, (ComponentArrayBase *)componentArray);
    }

    HashMap<ComponentArrayBase *> componentsArraysMap;

};

#endif // _COMPONENT_STORAGE_H_
