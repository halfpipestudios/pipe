#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include "memory.h"

struct MemoryStorage {
    void Initialize();
    void Terminate();

    void *AllocStaticMemory(u64 size, u64 align);
    void ClearMemory();
    
    void *AllocTemporalMemory(u64 size, u64 align);
    void BeginTemporalMemory();
    void EndTemporalMemory();
    
    void *AllocFrameMemory(u64 size, u64 align);
    void ClearFrameMemory();

private:  
    u64 lastMark;

    DoubleEndedStackAllocator memory;
    StackAllocator frame;
};

struct MemoryManager {
    static MemoryStorage *Get();
private:
    static MemoryStorage memoryStorage;
};

#endif // _MEMORY_MANAGER_H_
