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


// Slotmap -----------------------------------------------------------------------------------------

template <typename Type>
void Slotmap<Type>::Initialize(u32 size) {
    // Alloc memory for the arrays
    indices.Initialize(size);
    indices.size = size;
    data.Initialize(size);
    erase.Initialize(size);

    // initialize the freelist
    freelist = 0;
    generation = 0;
    for(u32 i = 0; i < size; ++i) {
        indices[i].id = i + 1;
        indices[i].gen = INVALID_KEY;
    } 
}

template <typename Type>
SlotmapKey Slotmap<Type>::Add(Type value) {
    
    // chek the slotmap is not full
    ASSERT(freelist < indices.capacity);

    // get a free slot and update the free list to the next free
    u32 freeIndex = freelist;
    SlotmapKey& freeKey = indices[freeIndex];
    freelist = freeKey.id;
    
    // set the id to the data next free position and gen to the generation
    ASSERT(generation < INVALID_KEY);
    freeKey.id = data.size;
    freeKey.gen = generation++;

    // push the value to the data array
    data.Push(value);
    erase.Push(freeIndex);

    return { freeIndex, freeKey.gen };
}

template <typename Type>
Type& Slotmap<Type>::Get(SlotmapKey key) {

    ASSERT(key.id < indices.capacity);
    
    // get the internal key and chek if is a valid key
    SlotmapKey askKey = indices[key.id];
    ASSERT(askKey.gen == key.gen);
    
    // if it is valid return the data
    return data[askKey.id];
}

template <typename Type>
void Slotmap<Type>::Remove(SlotmapKey key) {

    ASSERT(key.id < indices.capacity);

    // get the internal key and chek if is a valid key
    SlotmapKey& keyToRemove = indices[key.id];
    if(keyToRemove.gen != key.gen) {
        printf("key already deleted!!!\n");
        return;
    }

    // se the keyToRemove id to the next free and the freelist to the keyToRemove index
    u32 indexToRemove = keyToRemove.id;
    keyToRemove.id = freelist;
    keyToRemove.gen = INVALID_KEY;
    freelist = key.id;

    // remove the data and arease element
    if(indexToRemove < (data.size - 1)) { 
        // copy the last to the remove slot
        data[indexToRemove] = data[data.size - 1];
        erase[indexToRemove] = erase[erase.size - 1];
        // update the indice of the last value
        indices[erase[indexToRemove]].id = indexToRemove;
    }

    --data.size;
    --erase.size;        
    

    printf("key deleted!!!\n");
}

#endif
