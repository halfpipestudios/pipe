#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "common.h"
#include "cmp/base_cmp.h"

#include "data_structures.inl"
#include "serializer.h"
#include "tokenizer.h"

struct Input;

enum EntityFlags {
    ENTITY_STATIC    = 1 << 0,
    ENTITY_DYNAMIC   = 1 << 1,
    
    ENTITY_GROUNDED  = 1 << 2,
    ENTITY_COLLIDING = 1 << 3,
};

#define MAX_ENTITY_NAME 256
struct Entity_ : Serializable {

    char name[MAX_ENTITY_NAME];
    u32 flags;
    SlotmapKey key;
    
    inline void AddFlag(EntityFlags flag) { flags |= flag; }
    inline void RemoveFlag(EntityFlags flag) { flags &= ~flag; }
    inline void ClearFlags() { flags = 0; };
    inline bool HaveFlag(EntityFlags flag) { return (flags & flag) != 0; }

    void Serialize(Serializer *s) override;
    void Deserialize(Tokenizer *t) override;

    HashMap<SlotmapKey> componentsKeys;
    Array<u32> componentsIds;
};

#endif // _ENTITY_H_
