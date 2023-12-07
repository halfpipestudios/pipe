#ifndef _PHYSICS_CMP_H_
#define _PHYSICS_CMP_H_

struct PhysicsState {
    Vec3 pos { };
    Vec3 vel { };
    Vec3 acc { };

    f32 orientation { 0 };
    f32 angularVel  { 0 };
};

struct PhysicsCMP : CMP<PhysicsCMP> {

    void Initialize(Vec3 pos_, Vec3 vel_, Vec3 acc_) {
        physics.pos = pos_;
        physics.vel = vel_;
        physics.acc = acc_;
        lastPhysics = physics;
    }

    PhysicsState physics     { };
    PhysicsState lastPhysics { };
    Vec3 velXZ               { };
};

#endif // _PHYSICS_CMP_H_


