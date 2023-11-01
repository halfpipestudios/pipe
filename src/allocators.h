#ifndef _ALLOCATORS_H_
#define _ALLOCATORS_H_

#include "memory_manager.h"

struct FreeNode {
    FreeNode *next;
};

template <typename T>
struct ObjectAllocator {
    
    static_assert(sizeof(T) >= sizeof(FreeNode), "Object must be at least 8 bytes large");
    
    ObjectAllocator()
        : firstFree(nullptr) {}

    T *Alloc() {
        T *result = nullptr;
        if(firstFree != nullptr) {
            result = (T *)firstFree;
            firstFree = firstFree->next;
        } else {
            result = (T *)MemoryManager::Get()->AllocStaticMemory(sizeof(T), 8);
        }
        ASSERT(result != nullptr);
        return result;
    }
    
    void Free(T *object) {
        FreeNode *free = (FreeNode *)object;
        free->next = firstFree;
        firstFree = free;
    }

private:

    FreeNode *firstFree;
};


#endif 
