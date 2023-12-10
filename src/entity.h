#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "common.h"
#include "cmp/base_cmp.h"

#include "data_structures.h"

struct Input;

enum EntityFlags {
    ENTITY_STATIC    = 1 << 0,
    ENTITY_DYNAMIC   = 1 << 1,
    
    ENTITY_GROUNDED  = 1 << 2,
    ENTITY_COLLIDING = 1 << 3,
};

struct Entity_ {

    char *name;
    u32 flags;

    inline void AddFlag(EntityFlags flag) { flags |= flag; }
    inline void RemoveFlag(EntityFlags flag) { flags &= ~flag; }
    inline void ClearFlags() { flags = 0; };
    inline bool HaveFlag(EntityFlags flag) { return (flags & flag) != 0; }

    template <typename ComponentType>
    ComponentType* GetComponent() {
        return (ComponentType *)componentsPtrs.Get(ComponentType::GetID());
    }

    HashMap<CMPBase *> componentsPtrs;
};

#endif // _ENTITY_H_
