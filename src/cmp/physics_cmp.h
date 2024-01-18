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
        Write(s, "position", physics.pos);
        Write(s, "velocity", physics.vel);
        Write(s, "acceleration", physics.acc);
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        Vec3 pos_, vel_, acc_;
        ReadBeginObject(t, "physics");
        Read(t, "position", &pos_);
        Read(t, "velocity", &vel_);
        Read(t, "acceleration", &acc_);
        ReadEndObject(t);
        Initialize(pos_, vel_, acc_);
    };

};

#endif // _PHYSICS_CMP_H_


