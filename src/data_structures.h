#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#include "common.h"


/* ------------------------
      Circular Link list 
   ------------------------ */

/* NOTE: This is a Circular double link list and need to have a dummy object setup, pease dont forget to call dll_init */
#define ListInit(node) ((node)->prev = (node), (node)->next = (node))

#define ListInsertFront(node0, node1) \
    (node1)->prev = (node0); \
    (node1)->next = (node0)->next; \
    (node1)->prev->next = (node1); \
    (node1)->next->prev = (node1); \

#define ListInsertBack(node0, node1) \
    (node1)->prev = (node0)->prev; \
    (node1)->next = (node0); \
    (node1)->prev->next = (node1); \
    (node1)->next->prev = (node1); \

#define ListRemove(node) \
    (node)->prev->next = (node)->next; \
    (node)->next->prev = (node)->prev;

#define ListIsEmpty(dummy) \
    ((dummy)->next == (dummy) && (dummy)->prev == (dummy))

#define ListIsEnd(node, dummy) ((node) == (dummy))

#define ListIsFirst(node, dummy) ((node)->prev == (dummy))

#define ListIsLast(node, dummy) ((node)->next == (dummy))

#define ListGetBack(dummy) (dummy)->prev

#define ListGetTop(dummy) (dummy)->next

/* ------------------------
        Hash Map 
   ------------------------ */

template <typename Type>
struct HashMap {
    
    void Initialize(u32 size);

    void Add(u64 key, Type value);
    Type Get(u64 key);
    Type *GetPtr(u64 key);
    
    void Add(const char *key, Type value);
    Type *Get(const char *key);

    void Clear();

    struct HashElement {
        u32 id { 0 };
        Type value {};
    };

    u32 occupied { 0 };
    u32 capacity { 0 };
    u32 mask     { 0 };
    HashElement *elements { nullptr  };
    
    u32 seed { 123 };
};


template <typename Type>
struct Array {
    
    void Initialize(u32 size);

    Type *Push(Type value);

    Type& operator[] (u32 index){
        return data[index];
    }

    u32 capacity { 0 };
    u32 size     { 0 };
    Type *data { nullptr };
};

template <typename Type>
struct FrameArray {
    
    void Begin(u32 size);
    void End();

    Type *Push(Type value);

    Type& operator[] (u32 index){
        return data[index];
    }

    u32 capacity { 0 };
    u32 size     { 0 };
    Type *data { nullptr };
};

#define INVALID_KEY ((u64)-1)

struct SlotmapKey {
    u32 id   { 0 };
    u64 gen { INVALID_KEY };
};

template <typename Type>
struct Slotmap {

    void Initialize(u32 size);

    [[nodiscard]] SlotmapKey Add(Type value);
    Type& Get(SlotmapKey key);
    void Remove(SlotmapKey key);
    
    Array<SlotmapKey> indices {};
    Array<Type> data                {};
    Array<u32> erase                {};

    u32 freelist   { 0 };
    u32 generation { 0 };
};

#endif _HASH_MAP_H_
