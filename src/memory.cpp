#include "platform.h"
#include "memory.h"

extern Platform *gPlatform;

static inline u64 align_size_to_next_page_boundrie(u64 size) {
    u64 pageSize = gPlatform->GetPageSize();
    u64 numberOfPages = (size + (pageSize - 1)) / pageSize;
    u64 totalAllocSize =  numberOfPages*pageSize;
    return totalAllocSize;
}

/*----------------------------------------------*/
/*              Stack Allocator                 */
/*----------------------------------------------*/

void StackAllocator::Initialize(u64 size) {
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    this->memory = gPlatform->MemoryReserveAndCommit(totalAllocSize);
    this->used = 0;
    this->size = size;
}

void StackAllocator::Terminate() {
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    gPlatform->MemoryRelease(this->memory, totalAllocSize);
}

void *StackAllocator::Alloc(u64 size, u64 align) {
    u64 address = (u64)((u8 *)this->memory + this->used);
    u64 alignAddress = (address + (align - 1)) & ~(align - 1);
    u64 useForAlign = (alignAddress - address);
    
    ASSERT((this->used + useForAlign + size) <= this->size);
    
    this->used += useForAlign + size;
    return (void *)alignAddress;
}

void StackAllocator::Free() {
    this->used = 0;
}

/*----------------------------------------------*/
/*          Double Ended Stack Allocator        */
/*----------------------------------------------*/

u64 DoubleEndedStackAllocator::RemainingSize() {
    u64 bottom_used = (bottom - (u8 *)memory_bottom);
    u64 top_used = ((u8 *)memory_top - top);
    return size - (bottom_used + top_used);
}

void DoubleEndedStackAllocator::Initialize(u64 size) {
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    void *memory = gPlatform->MemoryReserveAndCommit(totalAllocSize);
    this->size = size;
    
    this->memory_bottom = memory;
    this->memory_top = (void *)((u8 *)memory + (size - 1));

    this->bottom = (u8 *)this->memory_bottom;
    this->top = (u8 *)this->memory_top;
}

void DoubleEndedStackAllocator::Terminate() {
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    gPlatform->MemoryRelease(this->memory_bottom, totalAllocSize);
}

void *DoubleEndedStackAllocator::AllocTop(u64 size, u64 align) {
    u64 alignAddress = ((u64)this->top) & ~(align - 1);
    u64 alignSize = (u64)this->top - alignAddress;
    ASSERT((size + alignSize) <= RemainingSize());
    
    this->top = (u8 *)(alignAddress - size);
    return (void *)alignAddress;
}

void *DoubleEndedStackAllocator::AllocBottom(u64 size, u64 align) {
    u64 alignAddress = (((u64)this->bottom) + (align - 1)) & ~(align - 1);
    u64 alignSize = alignAddress - (u64)this->bottom;
    ASSERT((size + alignSize) <= RemainingSize());
    
    this->bottom = (u8 *)(alignAddress + size);
    return (void *)alignAddress;
}

void DoubleEndedStackAllocator::FreeTop() {
    this->top = (u8 *)this->memory_top;
}

void DoubleEndedStackAllocator::FreeBottom() {
    this->bottom = (u8 *)this->memory_bottom;
}

/*----------------------------------------------*/
/*              Pooll Allocator                 */
/*----------------------------------------------*/

void PoolAllocator::Initialize(u64 blockSize, u64 numBlocks) {
    ASSERT(blockSize >= sizeof(FreeBlockNode));

    u64 size = blockSize * numBlocks;
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    this->memory = gPlatform->MemoryReserveAndCommit(totalAllocSize);

    this->size = size;
    this->blockSize = blockSize;
    
    for(u32 i = 0; i < numBlocks; ++i) {
        FreeBlockNode *block = (FreeBlockNode *)((u8 *)this->memory + (i * blockSize));
        if((i+1) == numBlocks) {
            block->next = nullptr;
        } else {
            block->next = (FreeBlockNode *)((u8 *)this->memory + ((i+1) * blockSize));
        }
    }
    freeList = (FreeBlockNode *)this->memory;
}

void PoolAllocator::Terminate() {
    u64 totalAllocSize =  align_size_to_next_page_boundrie(size);
    gPlatform->MemoryRelease(memory, totalAllocSize);
    freeList = nullptr;
}

void *PoolAllocator::AllocBlock() {
    void *result = (void *)this->freeList;
    this->freeList = this->freeList->next;
    return result;
}

void PoolAllocator::FreeBlock(void *block) {
    FreeBlockNode *freeBlock = (FreeBlockNode *)block;
    freeBlock->next = this->freeList;
    this->freeList = freeBlock;
}
