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
        BeginObject(s, "transform");
        
        Write(s, "position", 1);
        Write(s, "rotation", 2);
        Write(s, "scale", 3);
        
        EndObject(s);
    };
};

#endif // _TRANSFORM_CMP_H_
