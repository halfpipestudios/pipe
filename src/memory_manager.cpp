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
    lastMark = 0;
}

void MemoryStorage::Terminate() {
    memory.Terminate();
    frame.Terminate();
}

void *MemoryStorage::AllocStaticMemory(u64 size, u64 align) {
    void *result = memory.AllocTop(size, align);
    return result;
}

void MemoryStorage::ClearMemory() {
    memory.FreeTop();
    memory.FreeBottom();
}

void *MemoryStorage::AllocTemporalMemory(u64 size, u64 align) {
    void *result = memory.AllocBottom(size, align);
    return result;
}

void MemoryStorage::BeginTemporalMemory() {
    u8 *last_bottom = memory.bottom;
    u64 *mark = (u64 *)memory.AllocBottom(sizeof(u64), 8);
    *mark = (u64)last_bottom;
    lastMark = (u64)mark;

}

void MemoryStorage::EndTemporalMemory() {
    memory.bottom = (u8 *)lastMark;
    lastMark = *((u64 *)memory.bottom);
}

void *MemoryStorage::AllocFrameMemory(u64 size, u64 align) {
    void *result = frame.Alloc(size, align);
    return result;
}

void MemoryStorage::ClearFrameMemory() {
    frame.Free();
}
