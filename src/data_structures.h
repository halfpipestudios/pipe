#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#include "common.h"

template <typename Type, u32 Size>
struct StaticHashMap {
    StaticHashMap();
    void Add(u64 key, Type value);
    Type Get(u64 key);

    struct HashElement {
        u32 id { 0 };
        Type value {};
    };

    u32 occupied          { 0        };
    u32 capacity          { Size     };
    u32 mask              { Size - 1 };
    HashElement *elements { nullptr  };
    
    u32 seed { 123 };
};


template <typename Type, u32 Size>
struct StaticArray {
    StaticArray();
    Type *Push(Type value);

    u32 capacity { Size    };
    u32 size     { 0       };
    Type *data   { nullptr };
};



#endif
