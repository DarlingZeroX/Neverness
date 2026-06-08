#pragma once

#include <cmath>
#include <cstring>

namespace NN::Runtime::SRP
{
    // ========================================================================
    //  Minimal math types for SRP (no dependency on external math library)
    // ========================================================================

    struct Vector2
    {
        float x = 0.0f, y = 0.0f;
        Vector2() = default;
        Vector2(float x, float y) : x(x), y(y) {}
    };

    struct Vector3
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        Vector3() = default;
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
        Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
        Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
        float Length() const { return std::sqrt(x * x + y * y + z * z); }
        Vector3 Normalized() const { float l = Length(); return l > 0 ? *this * (1.0f / l) : Vector3{}; }
        float Dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
        Vector3 Cross(const Vector3& o) const {
            return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
        }
    };

    struct Vector4
    {
        float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
        Vector4() = default;
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    };

    // Column-major 4x4 matrix
    struct Matrix4x4
    {
        float m[16] = {};

        Matrix4x4()
        {
            // Identity
            m[0] = m[5] = m[10] = m[15] = 1.0f;
        }

        static Matrix4x4 Identity() { return Matrix4x4{}; }

        static Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
        {
            Vector3 f = (target - eye).Normalized();
            Vector3 r = f.Cross(up).Normalized();
            Vector3 u = r.Cross(f);

            Matrix4x4 result;
            result.m[0] = r.x;  result.m[4] = r.y;  result.m[8]  = r.z;  result.m[12] = -r.Dot(eye);
            result.m[1] = u.x;  result.m[5] = u.y;  result.m[9]  = u.z;  result.m[13] = -u.Dot(eye);
            result.m[2] = -f.x; result.m[6] = -f.y; result.m[10] = -f.z; result.m[14] = f.Dot(eye);
            result.m[3] = 0;    result.m[7] = 0;    result.m[11] = 0;    result.m[15] = 1.0f;
            return result;
        }

        static Matrix4x4 Perspective(float fovY, float aspect, float nearZ, float farZ)
        {
            float tanHalf = std::tan(fovY * 0.5f);
            Matrix4x4 result;
            std::memset(result.m, 0, sizeof(result.m));
            result.m[0] = 1.0f / (aspect * tanHalf);
            result.m[5] = 1.0f / tanHalf;
            result.m[10] = -(farZ + nearZ) / (farZ - nearZ);
            result.m[11] = -1.0f;
            result.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
            result.m[15] = 0.0f;
            return result;
        }

        static Matrix4x4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
        {
            Matrix4x4 result;
            std::memset(result.m, 0, sizeof(result.m));
            result.m[0] = 2.0f / (right - left);
            result.m[5] = 2.0f / (top - bottom);
            result.m[10] = -1.0f / (farZ - nearZ);
            result.m[12] = -(right + left) / (right - left);
            result.m[13] = -(top + bottom) / (top - bottom);
            result.m[14] = -nearZ / (farZ - nearZ);
            result.m[15] = 1.0f;
            return result;
        }

        Matrix4x4 operator*(const Matrix4x4& b) const
        {
            Matrix4x4 result;
            std::memset(result.m, 0, sizeof(result.m));
            for (int col = 0; col < 4; ++col)
                for (int row = 0; row < 4; ++row)
                    for (int k = 0; k < 4; ++k)
                        result.m[col * 4 + row] += m[k * 4 + row] * b.m[col * 4 + k];
            return result;
        }

        Vector4 operator*(const Vector4& v) const
        {
            return {
                m[0]*v.x + m[4]*v.y + m[8]*v.z  + m[12]*v.w,
                m[1]*v.x + m[5]*v.y + m[9]*v.z  + m[13]*v.w,
                m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.w,
                m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.w
            };
        }
    };

} // namespace NN::Runtime::SRP
