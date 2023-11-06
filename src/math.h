#ifndef _MATH_H_
#define _MATH_H_

#include "common.h"

#include <math.h>

#define EPSILON 0.0001f
#define VEC_EPSILON 0.000001f
#define PI 3.14159265359

#define TO_RAD(angle) ((angle)/180.0f * PI)

struct Vec2 {
    union {
        struct {
            f32 x, y;
        };
        f32 v[2];
    };

    Vec2() : x(0), y(0) {};
    Vec2(f32 _x, f32 _y) : x(_x), y(_y) {};

    f32 operator[](i32 index);

    Vec2 operator+(Vec2 &vec);
    Vec2 operator-(Vec2 &vec);
    Vec2 operator*(Vec2 &vec);
    Vec2 operator/(Vec2 &vec);

    void operator+=(Vec2 &vec);
    void operator-=(Vec2 &vec);
    void operator*=(Vec2 &vec);
    void operator/=(Vec2 &vec);

    Vec2 operator*(f32 val);
    Vec2 operator/(f32 val);
    void operator*=(f32 val);
    void operator/=(f32 val);

    f32 Dot(Vec2 &vec);
    f32 LenSq();
    f32 Len();
    void Normalize();
    Vec2 Normalized();


};

struct Vec3 {
    union {
        struct {
            f32 x, y, z;
        };
        f32 v[3];
    };

    Vec3() : x(0), y(0), z(0) {};
    Vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {};

    f32 operator[](i32 index);

    Vec3 operator+(Vec3 &vec);
    Vec3 operator-(Vec3 &vec);
    Vec3 operator*(Vec3 &vec);
    Vec3 operator/(Vec3 &vec);

    void operator+=(Vec3 &vec);
    void operator-=(Vec3 &vec);
    void operator*=(Vec3 &vec);
    void operator/=(Vec3 &vec);

    Vec3 operator*(f32 val);
    Vec3 operator/(f32 val);
    void operator*=(f32 val);
    void operator/=(f32 val);

    bool operator==(Vec3 &vec);

    f32 Dot(Vec3 &vec);
    Vec3 Cross(Vec3 &vec);
    f32 LenSq();
    f32 Len();
    void Normalize();
    Vec3 Normalized();


    static Vec3 Vec3::Lerp(Vec3 a, Vec3 b, f32 t);

};


Vec3 operator*(f32 val, Vec3 vec);

struct Vec4 {
    union {
        struct {
            f32 x, y, z, w;
        };
        f32 v[4];
    };

    Vec4() : x(0), y(0), z(0), w(0) {};
    Vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {};
    Vec4(Vec3 vec, f32 w_) : x(vec.x), y(vec.y), z(vec.z), w(w_) {}

    f32 operator[](i32 index);

    Vec4 operator+(Vec4 &vec);
    Vec4 operator-(Vec4 &vec);
    Vec4 operator*(Vec4 &vec);
    Vec4 operator/(Vec4 &vec);

    void operator+=(Vec4 &vec);
    void operator-=(Vec4 &vec);
    void operator*=(Vec4 &vec);
    void operator/=(Vec4 &vec);

    Vec4 operator*(f32 val);
    Vec4 operator/(f32 val);
    void operator*=(f32 val);
    void operator/=(f32 val);

    f32 Dot(Vec4 &vec);
    f32 LenSq();
    f32 Len();
    void Normalize();
    Vec4 Normalized();
};

struct iVec4 {
    union {
        struct {
            i32 x, y, z, w;
        };
        i32 v[4];
    };

    i32 operator[](i32 index);
};

struct Mat3 {
    union {
        struct {
            f32 m11, m12, m13;
            f32 m21, m22, m23;
            f32 m31, m32, m33;
        };
        f32 v[9];
    };

    Mat3()
         : m11(1), m12(0), m13(0),
           m21(0), m22(1), m23(0),
           m31(0), m32(0), m33(1) {}

    Mat3(f32 _m11, f32 _m12, f32 _m13,
         f32 _m21, f32 _m22, f32 _m23,
         f32 _m31, f32 _m32, f32 _m33)
         : m11(_m11), m12(_m12), m13(_m13),
           m21(_m21), m22(_m22), m23(_m23),
           m31(_m31), m32(_m32), m33(_m33) {}

    f32 *operator[](i32 index);

    Mat3 operator+(Mat3 &m);
    Mat3 operator*(f32 val);
    Mat3 operator*(Mat3 &m);
    Vec3 operator*(Vec3 &vec);
};

struct Mat4 {
    union {
        struct {
            f32 m11, m12, m13, m14;
            f32 m21, m22, m23, m24;
            f32 m31, m32, m33, m34;
            f32 m41, m42, m43, m44;
        };
        f32 v[16];
    };

    Mat4()
         : m11(1), m12(0), m13(0), m14(0),
           m21(0), m22(1), m23(0), m24(0),
           m31(0), m32(0), m33(1), m34(0),
           m41(0), m42(0), m43(0), m44(1) {}

    Mat4(f32 _m11, f32 _m12, f32 _m13, f32 _m14,
         f32 _m21, f32 _m22, f32 _m23, f32 _m24,
         f32 _m31, f32 _m32, f32 _m33, f32 _m34,
         f32 _m41, f32 _m42, f32 _m43, f32 _m44)
         : m11(_m11), m12(_m12), m13(_m13), m14(_m14),
           m21(_m21), m22(_m22), m23(_m23), m24(_m24),
           m31(_m31), m32(_m32), m33(_m33), m34(_m34),
           m41(_m41), m42(_m42), m43(_m43), m44(_m44) {}

    f32 *operator[](i32 index);

    Mat4 operator+(Mat4 &m);
    Mat4 operator*(f32 val);
    Mat4 operator*(Mat4 &m);
    Vec4 operator*(Vec4 &vec);

    static Vec3 TransformPoint(Mat4 mat, Vec3 &vec);
    static Vec3 TransformVector(Mat4 mat, Vec3 &vec);

    static Mat4 Frustum(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
    static Mat4 Perspective(f32 fov, f32 aspect, f32 znear, f32 zfar);
    static Mat4 Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
    static Mat4 LookAt(Vec3 position, Vec3 target, Vec3 up);
    static Mat4 Translate(f32 x, f32 y, f32 z);
    static Mat4 Translate(Vec3 pos);
    static Mat4 Scale(Vec3 scale);
    static Mat4 Scale(f32 x, f32 y, f32 z);
    static Mat4 RotateX(f32 angle);
    static Mat4 RotateY(f32 angle);
    static Mat4 RotateZ(f32 angle);

};

struct Quat {
    union {
        struct {
            f32 w, x, y, z;
        };
        f32 v[4];
    };

    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(f32 w, f32 x, f32 y, f32 z) : w(w), x(x), y(y), z(z) {}
    
    f32 operator[](i32 index);
    Mat4 ToMat4();

    static Quat Slerp(Quat a, Quat b, f32 t);
};

#endif
