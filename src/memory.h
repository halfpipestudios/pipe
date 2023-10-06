#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

struct StackAllocator {
    void *memory;
    u64 used;
    u64 size;
    u64 marker;

    void Initialize(u64 size);
    void Terminate();

    void *Alloc(u64 size, u64 align);
    void Free(u64 size);
   
    void Clear();
    void FreeToLastMarker();
}; 

#endif 
