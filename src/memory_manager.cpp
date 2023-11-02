#include "common.h"
#include "memory_manager.h"

MemoryStorage MemoryManager::memoryStorage;

MemoryStorage *MemoryManager::Get() {
    return &memoryStorage;
}

/*----------------------------------*/
/*          Memory Storage          */
/*----------------------------------*/

void MemoryStorage::Initialize() {
    memory.Initialize(MB(10));
    frame.Initialize(MB(10));
    lastMark = (u64)memory.bottom;
}

void MemoryStorage::Terminate() {
    memory.Terminate();
    frame.Terminate();
}

void *MemoryStorage::AllocStaticMemory(u64 size, u64 align) {
    void *result = memory.AllocTop(size, align);
    return result;
}

void MemoryStorage::ClearStaticMemory() {
    memory.FreeTop();
    memory.FreeBottom();
}

void *MemoryStorage::AllocTemporalMemory(u64 size, u64 align) {
    void *result = memory.AllocBottom(size, align);
    return result;
}

void MemoryStorage::BeginTemporalMemory() {
    u64 *markHeader = (u64 *)memory.AllocBottom(sizeof(u64), 1);
    *markHeader = lastMark;
    lastMark = (u64)markHeader;
}

void MemoryStorage::EndTemporalMemory() {
    memory.bottom = (u8 *)lastMark;
    lastMark = *((u64 *)lastMark);
}

void *MemoryStorage::AllocFrameMemory(u64 size, u64 align) {
    void *result = frame.Alloc(size, align);
    return result;
}

void MemoryStorage::ClearFrameMemory() {
    frame.Free();
}

u64 MemoryStorage::RemainingMemorySotrage() {
    return memory.RemainingSize();
}
