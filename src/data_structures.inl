#include "data_structures.h"
#include "memory_manager.h"
#include "utils.h"
#include <memory>

// StaticHashMap -----------------------------------------------------------------------------------------
template <typename Type, u32 Size>
StaticHashMap<Type, Size>::StaticHashMap() {
    elements = (HashElement *)MemoryManager::Get()->AllocStaticMemory(sizeof(HashElement) * capacity, 1);
    memset(elements, 0, sizeof(HashElement) * capacity);
}


template <typename Type, u32 Size>
void StaticHashMap<Type, Size>::Add(u64 key, Type value) {

    ASSERT(occupied + 1 <= capacity);

    u32 id = MurMur2(&key, sizeof(u64), seed);
    u32 index = (id & mask);

    if(elements[index].id == 0) {
        HashElement *element = elements + index;
        element->id = id;
        element->value = value;
        occupied++;
    } else {
        u32 nextIndex = index + 1;
        while(elements[nextIndex].id != 0 || nextIndex >= capacity) {
            nextIndex = (nextIndex + 1) % capacity;
        }
        HashElement *element = elements + nextIndex;
        element->id = id;
        element->value = value;
        occupied++;
    }

}

template <typename Type, u32 Size>
Type StaticHashMap<Type, Size>::Get(u64 key) {

    u32 id = MurMur2(&key, sizeof(u64), seed);
    u32 index = (id & mask);
    u32 counter = 0;
    
    while(((elements[index].id != 0 && elements[index].id != id) || elements[index].id == 0) && counter < capacity) {
        index = (index + 1) % capacity;
        ++counter;
    }

        // TODO: ------------------------ < or <= ?
    if(elements[index].id != 0 && counter < capacity) {
        return elements[index].value;
    }
    
    return {};
}


// StaticArray -----------------------------------------------------------------------------------------

template <typename Type, u32 Size>
StaticArray<Type, Size>::StaticArray() {
    data = (Type *)MemoryManager::Get()->AllocStaticMemory(sizeof(Type) * capacity, 1);
    memset(data, 0, sizeof(Type) * capacity);
}

template <typename Type, u32 Size>
Type *StaticArray<Type, Size>::Push(Type value) {

    ASSERT(size + 1 <= capacity);

    Type *element = new (data + size) Type;
    *element = value;

    ++size;
    return element;
}
