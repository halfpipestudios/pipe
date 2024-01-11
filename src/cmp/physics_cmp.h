#ifndef _PHYSICS_CMP_H_
#define _PHYSICS_CMP_H_

#include "base_cmp.h"

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
    Vec3 viewDir             { };


    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "physics");
        
        WriteBeginObject(s, "position");
        Write(s, "x", physics.pos.x);
        Write(s, "y", physics.pos.y);
        Write(s, "z", physics.pos.z);
        WriteEndObject(s);

        WriteBeginObject(s, "velocity");
        Write(s, "x", physics.vel.x);
        Write(s, "y", physics.vel.y);
        Write(s, "z", physics.vel.z);
        WriteEndObject(s);

        WriteBeginObject(s, "acceleration");
        Write(s, "x", physics.acc.x);
        Write(s, "y", physics.acc.y);
        Write(s, "z", physics.acc.z);
        WriteEndObject(s);
        
        
        WriteEndObject(s);
    };

};

#endif // _PHYSICS_CMP_H_


