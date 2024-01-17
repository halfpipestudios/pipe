#ifndef _TRANSFORM_CMP_H_
#define _TRANSFORM_CMP_H_

#include "algebra.h"

struct TransformCMP : CMP<TransformCMP> {

    void Initialize(Vec3 pos_, Vec3 rot_, Vec3 scale_) {
        pos = pos_;
        rot = rot_;
        scale = scale_;
    }

    Vec3 pos { };
    Vec3 rot { };
    Vec3 scale { 1, 1, 1 };
    Vec3 renderOffset { };
    inline Mat4 GetWorldMatrix() { 

        Quat rotX = Quat::AngleAxis( rot.x, Vec3(1, 0, 0));
        Quat rotY = Quat::AngleAxis(-rot.y, Vec3(0, 1, 0));
        Quat rotZ = Quat::AngleAxis( rot.z, Vec3(0, 0, 1));

        Quat rotQuat = rotX * rotY * rotZ;

        Vec3 x = rotQuat * Vec3(1, 0, 0);
        Vec3 y = rotQuat * Vec3(0, 1, 0);
        Vec3 z = rotQuat * Vec3(0, 0, 1);
        
        x = x * scale.x;
        y = y * scale.y;
        z = z * scale.z;

        Vec3 p = pos;

        return Mat4(
                x.x, y.x, z.x, p.x,
                x.y, y.y, z.y, p.y,
                x.z, y.z, z.z, p.z,
                0, 0, 0, 1);
    };


    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "transform");
        
        WriteBeginObject(s, "position");
        Write(s, "x", pos.x);
        Write(s, "y", pos.y);
        Write(s, "z", pos.z);
        WriteEndObject(s);

        WriteBeginObject(s, "rotation");
        Write(s, "x", rot.x);
        Write(s, "y", rot.y);
        Write(s, "z", rot.z);
        WriteEndObject(s);

        WriteBeginObject(s, "scale");
        Write(s, "x", scale.x);
        Write(s, "y", scale.y);
        Write(s, "z", scale.z);
        WriteEndObject(s);
        
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        Vec3 pos, rot, scale;

        ReadBeginObject(t, "transform");
        
        ReadBeginObject(t, "position");
        Read(t, "x", &pos.x);
        Read(t, "y", &pos.y);
        Read(t, "z", &pos.z);
        ReadEndObject(t);

        ReadBeginObject(t, "rotation");
        Read(t, "x", &rot.x);
        Read(t, "y", &rot.y);
        Read(t, "z", &rot.z);
        ReadEndObject(t);

        ReadBeginObject(t, "scale");
        Read(t, "x", &scale.x);
        Read(t, "y", &scale.y);
        Read(t, "z", &scale.z);
        ReadEndObject(t);
        
        ReadEndObject(t);

        Initialize(pos, rot, scale);
    };
};

#endif // _TRANSFORM_CMP_H_
