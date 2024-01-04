#ifndef _COMPONENT_STORAGE_H_
#define _COMPONENT_STORAGE_H_

#include "data_structures.inl"
#include "cmp/base_cmp.h"

#define COMPONENTS_TYPE_MAX_SIZE 64
#define COMPONENTS_ARRAY_MAX_SIZE 256

struct ComponentSlotmapBase {
    virtual void DestroyComponent(SlotmapKey key) = 0;
};

template <typename ComponentType>
struct ComponentSlotmap : ComponentSlotmapBase {
    Slotmap<ComponentType> components;
    void DestroyComponent(SlotmapKey key) override {
        components.Get(key).Terminate();
        components.Remove(key);
    }
};

struct ComponentStorage {

    void Initialize() {
        slotmaps.Initialize(COMPONENTS_TYPE_MAX_SIZE);
    }

    template <typename ComponentType>
    Array<ComponentType>& GetComponents() {
        ComponentSlotmap<ComponentType> *slotmap = (ComponentSlotmap<ComponentType> *)slotmaps.Get(ComponentType::GetID());
        return slotmap->components.data;
    }

    template <typename ComponentType>
    Slotmap<ComponentType>& GetComponentsSlotmap() {
        ComponentSlotmap<ComponentType> *slotmap = (ComponentSlotmap<ComponentType> *)slotmaps.Get(ComponentType::GetID());
        return slotmap->components;
    }

    ComponentSlotmapBase* GetComponentsSlotmapById(i32 id) {
        ComponentSlotmapBase*slotmap = slotmaps.Get(id);
        return slotmap;
    }

    template <typename ComponentType>
    void AddComponentType() {
        // alloc memory for the new slotmap and inilialize it
        void *buffer = MemoryManager::Get()->AllocStaticMemory(sizeof(ComponentSlotmap<ComponentType>), 8);
        ComponentSlotmap<ComponentType> *slotmap = new(buffer) ComponentSlotmap<ComponentType>;
        slotmap->components.Initialize(COMPONENTS_ARRAY_MAX_SIZE);
        // added to the slotmaps map
        slotmaps.Add(ComponentType::GetID(), (ComponentSlotmapBase *)slotmap);
    }


    HashMap<ComponentSlotmapBase *> slotmaps;
};

#endif // _COMPONENT_STORAGE_H_
