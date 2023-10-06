#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

struct StackAllocator {
    void *memory;
    u64 used;
    u64 size;

    void Initialize(u64 size);
    void Terminate();
    
    void *Alloc(u64 size, u64 align);
    void Free();

}; 

struct DoubleEndedStackAllocator {
    void *memory_bottom;
    void *memory_top;

    u8 *bottom;
    u8 *top;
    
    u64 size;
     
    void Initialize(u64 size);
    void Terminate();
    
    void *AllocTop(u64 size, u64 align);
    void *AllocBottom(u64 size, u64 align);

    void FreeTop();
    void FreeBottom();
    
    u64 RemainingSize();

};

struct PoolAllocator {
    
    void *memory;
    u64 blockSize;
    u64 size;
    
    struct FreeBlockNode {
        FreeBlockNode *next;
    };

    FreeBlockNode *freeList;

    void Initialize(u64 blockSize, u64 numBlocks);
    void Terminate();
    
    void *AllocBlock();
    void FreeBlock(void *block);
};

#endif 
