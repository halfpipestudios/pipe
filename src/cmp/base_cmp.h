#ifndef _COMPONENT_NEW_H_
#define _COMPONENT_NEW_H_

#include <data_structures.h>
#include "serializer.h"

struct CMPBase : Serializable {
    i32 id               { -1 };
    SlotmapKey entityKey { };

    void Serialize(Serializer *s) override { (void)s; };

protected:
    inline static i32 counter { 0 };
};

template <typename T>
struct CMP : CMPBase {
    
    CMP() { id = GetID(); }

    i32 static GetID() {
        static i32 id { ++counter };
        return id;
    }

};

#endif // _COMPONENT_NEW_H_
