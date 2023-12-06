#ifndef _COMPONENT_NEW_H_
#define _COMPONENT_NEW_H_

#include "common.h"

struct ComponentBase {
protected:
    inline static i32 counter { 0 };
};

template <typename T>
struct Component_ : ComponentBase {
    i32 static GetID() {
        static i32 id { ++counter };
        return id;
    }
};

struct AComponent : Component_<AComponent> {
    char *name = "componente A\n";
};

struct BComponent : Component_<BComponent> {
    char *name = "componente B\n";
};

struct CComponent : Component_<CComponent> {
    char *name = "componente C\n";
};

#endif // _COMPONENT_NEW_H_
