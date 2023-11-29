#include "math.h"

#include <math.h>
#include <float.h>

static inline f32 lerp(f32 a, f32 b, f32 t) {
    return a*(1-t) + b*t;
}

// ----------------Vec2---------------------
f32 Vec2::operator[](i32 index) {
    return v[index];
}

Vec2 Vec2::operator+(Vec2 &vec) {
    Vec2 result;
    result.x = x + vec.x;
    result.y = y + vec.y;
    return result;
}

Vec2 Vec2::operator-(Vec2 &vec) {
    Vec2 result;
    result.x = x - vec.x;
    result.y = y - vec.y;
    return result;
}

Vec2 Vec2::operator*(Vec2 &vec) {
    Vec2 result;
    result.x = x * vec.x;
    result.y = y * vec.y;
    return result;
}

Vec2 Vec2::operator/(Vec2 &vec) {
    Vec2 result;
    result.x = x / vec.x;
    result.y = y / vec.y;
    return result;
}

void Vec2::operator+=(Vec2 &vec) {
    x += vec.x;
    y += vec.y;
}

void Vec2::operator-=(Vec2 &vec) {
    x -= vec.x;
    y -= vec.y;
}

void Vec2::operator*=(Vec2 &vec) {
    x *= vec.x;
    y *= vec.y;
}

void Vec2::operator/=(Vec2 &vec) {
    x /= vec.x;
    y /= vec.y;
}

Vec2 Vec2::operator*(f32 val) {
    Vec2 result;
    result.x = x * val;
    result.y = y * val;
    return result;
}

Vec2 Vec2::operator/(f32 val) {
    Vec2 result;
    result.x = x / val;
    result.y = y / val;
    return result;
}

void Vec2::operator*=(f32 val) {
    x *= val;
    y *= val;
}

void Vec2::operator/=(f32 val) {
    x /= val;
    y /= val;
}

f32 Vec2::Dot(Vec2 &vec)
{
    return (x * vec.x) + (y * vec.y);
}

f32 Vec2::LenSq() {
    return (x * x) + (y * y);
}

f32 Vec2::Len() {
    return sqrtf((x * x) + (y * y));
}

void Vec2::Normalize() {
    f32 lenSqr = LenSq();
    if(lenSqr < FLT_EPSILON) {
        ASSERT(!"ERROR trying to normalize a zero len vector");
    }

    f32 len = sqrtf(lenSqr);
    x /= len;
    y /= len;
}

Vec2 Vec2::Normalized() {
    f32 lenSqr = LenSq();
    if(lenSqr < FLT_EPSILON) {
        ASSERT(!"ERROR trying to normalize a zero len vector");
    }

    f32 len = sqrtf(lenSqr);
    Vec2 result;
    result.x = x / len;
    result.y = y / len;
    return result;
}

// -----------------------------------------


// ----------------Vec3---------------------
f32 Vec3::operator[](i32 index) {
    return v[index];
}

Vec3 Vec3::operator+(Vec3 &vec) {
    Vec3 result;
    result.x = x + vec.x;
    result.y = y + vec.y;
    result.z = z + vec.z;
    return result;
}

Vec3 Vec3::operator-(Vec3 &vec) {
    Vec3 result;
    result.x = x - vec.x;
    result.y = y - vec.y;
    result.z = z - vec.z;
    return result;
}

Vec3 Vec3::operator*(Vec3 &vec) {
    Vec3 result;
    result.x = x * vec.x;
    result.y = y * vec.y;
    result.z = z * vec.z;
    return result;
}

Vec3 Vec3::operator/(Vec3 &vec) {
    Vec3 result;
    result.x = x / vec.x;
    result.y = y / vec.y;
    result.z = z / vec.z;
    return result;
}

void Vec3::operator+=(Vec3 &vec) {
    x += vec.x;
    y += vec.y;
    z += vec.z;
}

void Vec3::operator-=(Vec3 &vec) {
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
}

void Vec3::operator*=(Vec3 &vec) {
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
}

void Vec3::operator/=(Vec3 &vec) {
    x /= vec.x;
    y /= vec.y;
    z /= vec.z;
}

Vec3 Vec3::operator*(f32 val) {
    Vec3 result;
    result.x = x * val;
    result.y = y * val;
    result.z = z * val;
    return result;
}

Vec3 Vec3::operator/(f32 val) {
    Vec3 result;
    result.x = x / val;
    result.y = y / val;
    result.z = z / val;
    return result;
}

void Vec3::operator*=(f32 val) {
    x *= val;
    y *= val;
    z *= val;
}

void Vec3::operator/=(f32 val) {
    x /= val;
    y /= val;
    z /= val;
}

bool Vec3::operator==(Vec3 &vec) {
    Vec3 diff = {*this - vec};
    return  diff.LenSq() < VEC_EPSILON;
}

Vec3 operator*(f32 val, Vec3 vec) {
    Vec3 result;
    result.x = vec.x * val;
    result.y = vec.y * val;
    result.z = vec.z * val;
    return result;
}

f32 Vec3::Dot(Vec3 &vec)
{
    return (x * vec.x) + (y * vec.y) + (z * vec.z);
}

Vec3 Vec3::Cross(Vec3 &vec) {
    return Vec3(
        y * vec.z - z * vec.y, 
        z * vec.x - x * vec.z, 
        x * vec.y - y * vec.x 
    );
}

f32 Vec3::LenSq() {
    return (x * x) + (y * y) + (z * z);
}

f32 Vec3::Len() {
    return sqrtf((x * x) + (y * y) + (z * z));
}

void Vec3::Normalize() {
    f32 lenSqr = LenSq();
    if(lenSqr == 0.0f) {
        //ASSERT(!"ERROR trying to normalize a zero len vector");
        return;
    }

    f32 len = sqrtf(lenSqr);
    x /= len;
    y /= len;
    z /= len;
}

Vec3 Vec3::Normalized() {
    f32 lenSqr = LenSq();
    if(lenSqr == 0.0f) {
        //ASSERT(!"ERROR trying to normalize a zero len vector");
        return {};
    }
    else
    {
        f32 len = sqrtf(lenSqr);
        Vec3 result;
        result.x = x / len;
        result.y = y / len;
        result.z = z / len;
        return result;
    }
}

Vec3 Vec3::Lerp(Vec3 a, Vec3 b, f32 t) {
    Vec3 result;
    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);
    result.z = lerp(a.z, b.z, t);
    return result;
}

// -----------------------------------------



// ----------------Vec4---------------------
f32 Vec4::operator[](i32 index) {
    return v[index];
}

Vec4 Vec4::operator+(Vec4 &vec) {
    Vec4 result;
    result.x = x + vec.x;
    result.y = y + vec.y;
    result.z = z + vec.z;
    result.w = w + vec.w;
    return result;
}

Vec4 Vec4::operator-(Vec4 &vec) {
    Vec4 result;
    result.x = x - vec.x;
    result.y = y - vec.y;
    result.z = z - vec.z;
    result.w = w - vec.w;
    return result;
}

Vec4 Vec4::operator*(Vec4 &vec) {
    Vec4 result;
    result.x = x * vec.x;
    result.y = y * vec.y;
    result.z = z * vec.z;
    result.w = w * vec.w;
    return result;
}

Vec4 Vec4::operator/(Vec4 &vec) {
    Vec4 result;
    result.x = x / vec.x;
    result.y = y / vec.y;
    result.z = z / vec.z;
    result.w = w / vec.w;
    return result;
}

void Vec4::operator+=(Vec4 &vec) {
    x += vec.x;
    y += vec.y;
    z += vec.z;
    w += vec.w;
}

void Vec4::operator-=(Vec4 &vec) {
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    w -= vec.w;
}

void Vec4::operator*=(Vec4 &vec) {
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    w *= vec.w;
}

void Vec4::operator/=(Vec4 &vec) {
    x /= vec.x;
    y /= vec.y;
    z /= vec.z;
    w /= vec.w;
}

Vec4 Vec4::operator*(f32 val) {
    Vec4 result;
    result.x = x * val;
    result.y = y * val;
    result.z = z * val;
    result.w = w * val;
    return result;
}

Vec4 Vec4::operator/(f32 val) {
    Vec4 result;
    result.x = x / val;
    result.y = y / val;
    result.z = z / val;
    result.w = w / val;
    return result;
}

void Vec4::operator*=(f32 val) {
    x *= val;
    y *= val;
    z *= val;
    w *= val;
}

void Vec4::operator/=(f32 val) {
    x /= val;
    y /= val;
    z /= val;
    w /= val;
}

f32 Vec4::Dot(Vec4 &vec)
{
    return (x * vec.x) + (y * vec.y) + (z * vec.z) + (w * vec.w);
}

f32 Vec4::LenSq() {
    return (x * x) + (y * y) + (z * z) + (w * w);
}

f32 Vec4::Len() {
    return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
}

void Vec4::Normalize() {
    f32 lenSqr = LenSq();
    if(lenSqr < FLT_EPSILON) {
        ASSERT(!"ERROR trying to normalize a zero len vector");
    }

    f32 len = sqrtf(lenSqr);
    x /= len;
    y /= len;
    z /= len;
    w /= len;
}

Vec4 Vec4::Normalized() {
    f32 lenSqr = LenSq();
    if(lenSqr < FLT_EPSILON) {
        ASSERT(!"ERROR trying to normalize a zero len vector");
    }

    f32 len = sqrtf(lenSqr);
    Vec4 result;
    result.x = x / len;
    result.y = y / len;
    result.z = z / len;
    result.w = w / len;
    return result;
}

// -----------------------------------------

i32 iVec4::operator[](i32 index) {
    return v[index];
}


// ----------------Mat3---------------------
f32 *Mat3::operator[](i32 index) {
    return &(v[index * 3]);
}

Mat3 Mat3::operator+(Mat3 &m) {
    return Mat3(
        m11 + m.m11, m12 + m.m12, m13 + m.m13,
        m21 + m.m21, m22 + m.m22, m23 + m.m23,
        m31 + m.m31, m32 + m.m32, m33 + m.m33);
}

Mat3 Mat3::operator*(f32 val) {
    return Mat3(
        m11 * val, m12 * val, m13 * val,
        m21 * val, m22 * val, m23 * val,
        m31 * val, m32 * val, m33 * val);
}

Mat3 Mat3::operator*(Mat3 &m) {
    Mat3 result;
    for(i32 row = 0; row < 3; ++row) {
        for(i32 col = 0; col < 3; ++col) {
            result[row][col] =
                v[row * 3 + 0] * m.v[0 * 3 + col] +
                v[row * 3 + 1] * m.v[1 * 3 + col] +
                v[row * 3 + 2] * m.v[2 * 3 + col];
        }
    }
    return result;
}


Vec3 Mat3::operator*(Vec3 &vec) {
    Vec3 result;
    for(i32 row = 0; row < 3; ++row) {
        result.v[row] = v[row * 3 + 0] * vec.x + v[row * 3 + 1] * vec.y + v[row * 3 + 2] * vec.z;
    }
    return result;
}

// -----------------------------------------

// ----------------Mat4---------------------
f32 *Mat4::operator[](i32 index) {
    return &(v[index * 4]);
}


Mat4 Mat4::operator+(Mat4 &m) {
    return Mat4(
        m11 + m.m11, m12 + m.m12, m13 + m.m13, m14 + m.m14,
        m21 + m.m21, m22 + m.m22, m23 + m.m23, m24 + m.m24,
        m31 + m.m31, m32 + m.m32, m33 + m.m33, m34 + m.m34,
        m41 + m.m41, m42 + m.m42, m43 + m.m43, m44 + m.m44);
}

Mat4 Mat4::operator*(f32 val) {
    return Mat4(
        m11 * val, m12 * val, m13 * val, m14 * val,
        m21 * val, m22 * val, m23 * val, m24 * val,
        m31 * val, m32 * val, m33 * val, m34 * val,
        m41 * val, m42 * val, m43 * val, m44 * val);
}

Mat4 Mat4::operator*(Mat4 &m) {
    Mat4 result;
    for(i32 row = 0; row < 4; ++row) {
        for(i32 col = 0; col < 4; ++col) {
            result[row][col] =
                v[row * 4 + 0] * m.v[0 * 4 + col] +
                v[row * 4 + 1] * m.v[1 * 4 + col] +
                v[row * 4 + 2] * m.v[2 * 4 + col] +
                v[row * 4 + 3] * m.v[3 * 4 + col];
        }
    }
    return result;
}


Vec4 Mat4::operator*(Vec4 &vec) {
    Vec4 result;
    for(i32 row = 0; row < 4; ++row) {
        result.v[row] = v[row * 4 + 0] * vec.x + v[row * 4 + 1] * vec.y + v[row * 4 + 2] * vec.z + v[row * 4 + 3] * vec.w;
    }
    return result;
}

Vec3 Mat4::TransformPoint(Mat4 mat, Vec3 &vec) {
    Vec4 result = mat * Vec4(vec, 1.0f);
    return {result.x, result.y, result.z};
}

Vec3 Mat4::TransformVector(Mat4 mat, Vec3 &vec) {
    Vec4 result = mat * Vec4(vec, 0.0f);
    return {result.x, result.y, result.z};
}

Mat4 Mat4::Frustum(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    if (l == r || t == b || n == f) {
        ASSERT(!"WARNING: Trying to create invalid frustum\n");
    }

    return Mat4(
        (2.0f * n) / (r - l), 0, -(r + l) / (r - l), 0,
        0, (2.0f * n) / (t - b), -(t + b) / (t - b), 0,
        0, 0, f / (f - n), -(f * n) / (f - n),
        0, 0, 1, 0);
}

Mat4 Mat4::Perspective(f32 fov, f32 aspect, f32 znear, f32 zfar) {
    f32 ymax = znear * tanf(fov * 3.14159265359f / 360.0f);
    f32 xmax = ymax * aspect;
    return Frustum(-xmax, xmax, -ymax, ymax, znear, zfar);
}

Mat4 Mat4::Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    if (l == r || t == b || n == f) {
        ASSERT(!"INVALID_CODE_PATH");
    }
    
    return Mat4(
       2.0f / (r - l), 0, 0, -((r + l) / (r - l)),
       0, 2.0f / (t - b), 0, -(t + b) / (t - b),
       0, 0, 1.0f / (f - n), -(n / (f - n)),
       0, 0, 0, 1);

}

Mat4 Mat4::LookAt(Vec3 position, Vec3 target, Vec3 up) {
    Vec3 zaxis = (target - position).Normalized();
    Vec3 xaxis = up.Cross(zaxis).Normalized();
    Vec3 yaxis = zaxis.Cross(xaxis).Normalized();
    return Mat4(
        xaxis.x, xaxis.y, xaxis.z, -xaxis.Dot(position), 
        yaxis.x, yaxis.y, yaxis.z, -yaxis.Dot(position), 
        zaxis.x, zaxis.y, zaxis.z, -zaxis.Dot(position),
        0,       0,       0,       1);
}

Mat4 Mat4::Translate(Vec3 pos) {
    return Translate(pos.x, pos.y, pos.z);
}

Mat4 Mat4::Translate(f32 x, f32 y, f32 z) {
    return Mat4(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1);
}

Mat4 Mat4::Scale(Vec3 scale) { 
    return Scale(scale.x, scale.y, scale.z);
}

Mat4 Mat4::Scale(f32 x, f32 y, f32 z) {
    return Mat4(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1);
}

Mat4 Mat4::RotateX(f32 angle) {
    return Mat4 (
            1,            0,           0,  0,
            0,  cosf(angle), sinf(angle),  0,
            0, -sinf(angle), cosf(angle),  0,
            0,            0,           0,  1);
}

Mat4 Mat4::RotateY(f32 angle) {
    return Mat4(
             cosf(angle), 0, -sinf(angle), 0,
                       0, 1,            0, 0,
             sinf(angle), 0,  cosf(angle), 0,
                       0, 0,            0, 1);
}

Mat4 Mat4::RotateZ(f32 angle) {
    return Mat4(
             cosf(angle), sinf(angle), 0, 0,
            -sinf(angle), cosf(angle), 0, 0,
                       0,           0, 1, 0,
                       0,           0, 0, 1);
}

Mat4 Mat4::Rotate(Vec3 rotate) {
    return RotateX(rotate.x) * RotateY(rotate.y) * RotateZ(rotate.z) ;
}

// -----------------------------------------

// ----------------Quat---------------------
f32 Quat::operator[](i32 index) {
    return v[index];
}

Quat Quat::Slerp(Quat a, Quat b, f32 t) {

    f32 k0, k1;

    f32 cos_omega = a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
    if(cos_omega < 0.0f) {
        b.w = -b.w;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        cos_omega = -cos_omega;
    }
    
    if(cos_omega > 0.9999f) {
        k0 = (1 - t);
        k1 = t;
    } else {
        
        f32 sin_omega = sqrtf(1.0f - cos_omega*cos_omega);
        f32 omega = atan2(sin_omega, cos_omega);
        f32 one_over_sin_omega = 1.0f / sin_omega;
        k0 = sin((1.0f - t) * omega) * one_over_sin_omega;
        k1 = sin(t * omega) * one_over_sin_omega;
    }

    Quat result;

    result.w = a.w*k0 + b.w*k1;
    result.x = a.x*k0 + b.x*k1;
    result.y = a.y*k0 + b.y*k1;
    result.z = a.z*k0 + b.z*k1;

    return result;
}

Mat4 Quat::ToMat4() {

    Mat4 result;

    result.v[0] = 1 - 2*y*y - 2*z*z;
    result.v[1] = 2 * (x*y - w*z);
    result.v[2] = 2 * (x*z + w*y);
    result.v[3] = 0;

    result.v[4] = 2 * (x*y + w*z);
    result.v[5] = 1 - 2*x*x - 2*z*z;
    result.v[6] = 2 * (y*z - w*x);
    result.v[7] = 0;

    result.v[8]  = 2 * (x*z - w*y);
    result.v[9]  = 2 * (y*z + w*x);
    result.v[10] = 1 - 2*x*x - 2*y*y;
    result.v[11] = 0;

    result.v[12] = 0;
    result.v[13] = 0;
    result.v[14] = 0;
    result.v[15] = 1;

    return result;
}

Quat Quat::operator*(float f) {
    Quat result; 
    result.w = this->w * f;
    result.x = this->x * f;
    result.y = this->y * f;
    result.z = this->z * f;
    return result;
}

Quat Quat::operator/(float f) {
    Quat result = *this * (1.0f/f); 
    return result;
}

Quat Quat::operator+(Quat &q) {
    Quat result; 
    result.w = this->w + q.w;
    result.x = this->x + q.x;
    result.y = this->y + q.y;
    result.z = this->z + q.z;
    return result;
}

// -----------------------------------------
