#ifndef _COMPONENT_NEW_H_
#define _COMPONENT_NEW_H_

struct Entity_;

struct CMPBase {
protected:
    inline static i32 counter { 0 };
};

template <typename T>
struct CMP : CMPBase {
    i32 static GetID() {
        static i32 id { ++counter };
        return id;
    }
    Entity_ *entity;
};

#endif // _COMPONENT_NEW_H_