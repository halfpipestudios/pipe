#ifndef _COMPONENT_STORAGE_H_
#define _COMPONENT_STORAGE_H_

#include "data_structures.c"
#include "components_new.h"

#define COMPONENTS_ARRAY_MAP_SIZE 64
#define COMPONENTS_ARRAY_MAX_SIZE 256

struct ComponentArrayBase {};

template <typename ComponentType>
struct ComponentArray : ComponentArrayBase {
    StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE> components;
};

struct ComponentStorage {

    template <typename ComponentType>
    StaticArray<ComponentType, COMPONENTS_ARRAY_MAX_SIZE>* GetComponents() {
        i32 id = ComponentType::GetID(); 
        ComponentArray<ComponentType> *componentArray = (ComponentArray<ComponentType> *)componentsArraysMap.Get(id);
        return &componentArray->components;
    }

    template <typename ComponentType>
    void AddComponentType() {
        i32 id = ComponentType::GetID(); 
        void *buffer = MemoryManager::Get()->AllocStaticMemory(sizeof(ComponentArray<ComponentType>), 1);
        ComponentArrayBase *componentArray = new(buffer) ComponentArray<ComponentType>;
        componentsArraysMap.Add(id, componentArray);
    }

    StaticHashMap<ComponentArrayBase *, COMPONENTS_ARRAY_MAP_SIZE> componentsArraysMap;

};

#endif // _COMPONENT_STORAGE_H_
