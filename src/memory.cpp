#include "platform.h"
#include "memory.h"

extern Platform *gPlatform;

/*----------------------------------------------*/
/*              Stack Allocator                 */
/*----------------------------------------------*/

void StackAllocator::Initialize(u64 size) {
    u64 pageSize = gPlatform->GetPageSize();
    u64 numberOfPages = (size + (pageSize - 1)) / pageSize;
    u64 totalAllocSize =  numberOfPages*pageSize;
    this->memory = gPlatform->MemoryReserveAndCommit(totalAllocSize);
    this->used = 0;
    this->size = size;
    this->marker = 0;
}

void StackAllocator::Terminate() {
    u64 pageSize = gPlatform->GetPageSize();
    u64 numberOfPages = (size + (pageSize - 1)) / pageSize;
    u64 totalAllocSize =  numberOfPages*pageSize;
    gPlatform->MemoryRelease(this->memory, totalAllocSize);
}

void *StackAllocator::Alloc(u64 size, u64 align) {
    u64 used_align = (this->used + (align - 1)) & ~(align - 1);
    ASSERT(used_align + size <= this->size);
    this->marker = this->used;
    this->used = used_align + size;
    return (void *)((u8 *)this->memory + used_align);
}

void StackAllocator::Free(u64 size) {
    ASSERT(size <= this->used);
    this->used -= size;
}

void StackAllocator::Clear() {
    this->used = 0;
}

void StackAllocator::FreeToLastMarker() {
    this->used = this->marker;
}
