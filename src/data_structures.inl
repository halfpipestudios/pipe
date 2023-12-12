#ifndef _HASH_MAP_INL_
#define _HASH_MAP_INL_

#include "data_structures.h"
#include "memory_manager.h"
#include "utils.h"
#include <memory>

// HashMap -----------------------------------------------------------------------------------------
template <typename Type>
void HashMap<Type>::Initialize(u32 size) {
    ASSERT(IS_POWER_OF_TWO(size));
    capacity = size;
    mask     = (size - 1);
    occupied = 0;

    elements = (HashElement *)MemoryManager::Get()->AllocStaticMemory(sizeof(HashElement) * capacity, 8);
    memset(elements, 0, sizeof(HashElement) * capacity);
}


template <typename Type>
void HashMap<Type>::Clear() {
    occupied = 0;
    memset(elements, 0, sizeof(HashElement) * capacity);
}


template <typename Type>
void HashMap<Type>::Add(u64 key, Type value) {

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

template <typename Type>
void HashMap<Type>::Add(const char *key, Type value) {

    ASSERT(occupied + 1 <= capacity);

    u32 id = MurMur2(key, strlen(key), seed);
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

template <typename Type>
Type HashMap<Type>::Get(u64 key) {

    u32 id = MurMur2(&key, sizeof(u64), seed);
    u32 index = (id & mask);

    u32 counter = 0;
    while((elements[index].id != 0 && elements[index].id != id) && counter < capacity) {
        index = (index + 1) % capacity;
        ++counter;
    }

    if(elements[index].id != 0 && counter <= capacity) {
        return elements[index].value;
    }
    
    return {};
}

template <typename Type>
Type *HashMap<Type>::GetPtr(u64 key) {
    u32 id = MurMur2(&key, sizeof(u64), seed);
    u32 index = (id & mask);
    u32 counter = 0;
    
    while((elements[index].id != 0 && elements[index].id != id) && counter < capacity) {
        index = (index + 1) % capacity;
        ++counter;
    }

    if(elements[index].id != 0 && counter <= capacity) {
        return &elements[index].value;
    }

    return nullptr;    
}

template <typename Type>
Type *HashMap<Type>::Get(const char *key) {
    u32 id = MurMur2(key, strlen(key), seed);
    u32 index = (id & mask);
    u32 counter = 0;
    
    while((elements[index].id != 0 && elements[index].id != id) && counter < capacity) {
        index = (index + 1) % capacity;
        ++counter;
    }

    if(elements[index].id != 0 && counter <= capacity) {
        return &elements[index].value;
    }

    return nullptr;    
}

// Array -----------------------------------------------------------------------------------------

template <typename Type>
void Array<Type>::Initialize(u32 size) {
    capacity = size;
    data = (Type *)MemoryManager::Get()->AllocStaticMemory(sizeof(Type) * capacity, 8);
    memset(data, 0, sizeof(Type) * capacity);
}

template <typename Type>
Type *Array<Type>::Push(Type value) {
    ASSERT(size + 1 <= capacity);
    Type *element = new (data + size) Type;
    *element = value;
    ++size;
    return element;
}

#endif
