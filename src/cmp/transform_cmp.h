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
        return Mat4::Translate(pos) * Mat4::Rotate(rot) * Mat4::Scale(scale); 
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
