#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include "memory.h"

struct MemoryStorage {
    
    void Initialize();
    void Terminate();

    void *AllocStaticMemory(u64 size, u64 align);
    void ClearStaticMemory();
    
    void *AllocTemporalMemory(u64 size, u64 align);
    void BeginTemporalMemory();
    void EndTemporalMemory();

    u64 RemainingMemorySotrage();
    
    void *AllocFrameMemory(u64 size, u64 align);
    void ClearFrameMemory();

    inline DoubleEndedStackAllocator *GetDoubleStack() { return &memory; }

private:  
    DoubleEndedStackAllocator memory;
    u64 lastMark;
    
    StackAllocator frame;
};

struct MemoryManager {
    static MemoryStorage *Get();
private:
    static MemoryStorage memoryStorage;
};

#endif // _MEMORY_MANAGER_H_
