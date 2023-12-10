#ifndef _COMPONENT_STORAGE_H_
#define _COMPONENT_STORAGE_H_

#include "data_structures.inl"
#include "cmp/base_cmp.h"

#define COMPONENTS_ARRAY_MAP_SIZE 64
#define COMPONENTS_ARRAY_MAX_SIZE 256

struct ComponentArrayBase {};

template <typename ComponentType>
struct ComponentArray : ComponentArrayBase {
    Array<ComponentType> components;
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
