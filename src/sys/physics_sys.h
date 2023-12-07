#ifndef _PHYSICS_SYS_H_
#define _PHYSICS_SYS_H_

template<typename EM>
struct PhysicsSys {
    void Update(EM& em, f32 dt);
    void PostUpdate(EM& em, f32 dt);
};

#endif // _PHYSICS_SYS_H_
