/******************************************************************************
 * File: MathTypes.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Basic math types (simplified version without GLM)
 * Note: In production, use GLM library
 ******************************************************************************/

#pragma once

#include <cmath>
#include <iostream>

namespace MyEngine {

/**
 * @brief 2D vector
 */
struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 Normalized() const {
        float len = Length();
        return len > 0 ? Vec2(x / len, y / len) : Vec2();
    }
};

/**
 * @brief 3D vector
 */
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 Normalized() const {
        float len = Length();
        return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3();
    }

    static Vec3 Cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static float Dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    
    // Linear interpolation
    static Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
        return Vec3(
            a.x + t * (b.x - a.x),
            a.y + t * (b.y - a.y),
            a.z + t * (b.z - a.z)
        );
    }
};

/**
 * @brief 4D vector
 */
struct Vec4 {
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

/**
 * @brief 3x3 Matrix (column-major)
 */
struct Mat3 {
    float m[9];

    Mat3() {
        Identity();
    }

    void Identity() {
        for (int i = 0; i < 9; i++) m[i] = 0;
        m[0] = m[4] = m[8] = 1.0f;
    }
};

/**
 * @brief 4x4 Matrix (column-major)
 */
struct Mat4 {
    float m[16];

    Mat4() {
        Identity();
    }

    void Identity() {
        for (int i = 0; i < 16; i++) m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Mat4 Translation(const Vec3& t) {
        Mat4 result;
        result.m[12] = t.x;
        result.m[13] = t.y;
        result.m[14] = t.z;
        return result;
    }

    static Mat4 Scale(const Vec3& s) {
        Mat4 result;
        result.m[0] = s.x;
        result.m[5] = s.y;
        result.m[10] = s.z;
        return result;
    }

    static Mat4 Rotation(float angle, const Vec3& axis) {
        Mat4 result;
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;
        Vec3 a = axis.Normalized();

        result.m[0] = t * a.x * a.x + c;
        result.m[1] = t * a.x * a.y + a.z * s;
        result.m[2] = t * a.x * a.z - a.y * s;

        result.m[4] = t * a.x * a.y - a.z * s;
        result.m[5] = t * a.y * a.y + c;
        result.m[6] = t * a.y * a.z + a.x * s;

        result.m[8] = t * a.x * a.z + a.y * s;
        result.m[9] = t * a.y * a.z - a.x * s;
        result.m[10] = t * a.z * a.z + c;
        return result;
    }

    static Mat4 Perspective(float fovRad, float aspect, float nearClip, float farClip) {
        Mat4 result;
        float tanHalfFov = std::tan(fovRad / 2.0f);
        
        for (int i = 0; i < 16; i++) result.m[i] = 0.0f;
        
        result.m[0] = 1.0f / (aspect * tanHalfFov);
        result.m[5] = 1.0f / tanHalfFov;
        result.m[10] = -(farClip + nearClip) / (farClip - nearClip);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
        
        return result;
    }

    static Mat4 Ortho(float left, float right, float bottom, float top, float nearClip, float farClip) {
        Mat4 result;
        result.m[0] = 2.0f / (right - left);
        result.m[5] = 2.0f / (top - bottom);
        result.m[10] = -2.0f / (farClip - nearClip);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -(farClip + nearClip) / (farClip - nearClip);
        result.m[15] = 1.0f;
        return result;
    }

    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                float sum = 0;
                for (int i = 0; i < 4; i++) {
                    sum += m[i * 4 + row] * other.m[col * 4 + i];
                }
                result.m[col * 4 + row] = sum;
            }
        }
        return result;
    }
    
    Vec4 operator*(const Vec4& v) const {
        Vec4 result;
        result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
        result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
        result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
        result.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
        return result;
    }

    Mat4 Inverted() const {
        // Simplified inverse for translation + rotation matrices only (orthonormal)
        Mat4 result;
        // Transpose of rotation part
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i * 4 + j] = m[j * 4 + i];
            }
        }
        // New translation: -R^T * t
        Vec3 t(m[12], m[13], m[14]);
        result.m[12] = -(result.m[0] * t.x + result.m[4] * t.y + result.m[8] * t.z);
        result.m[13] = -(result.m[1] * t.x + result.m[5] * t.y + result.m[9] * t.z);
        result.m[14] = -(result.m[2] * t.x + result.m[6] * t.y + result.m[10] * t.z);
        return result;
    }
};

/**
 * @brief Quaternion for rotations
 */
struct Quat {
    float x, y, z, w;

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    static Quat Identity() {
        return Quat(0, 0, 0, 1);
    }
    
    // Normalize quaternion
    Quat Normalized() const {
        float len = sqrtf(x * x + y * y + z * z + w * w);
        if (len < 0.0001f)
            return Quat(0, 0, 0, 1);
        return Quat(x / len, y / len, z / len, w / len);
    }
    
    // Spherical linear interpolation
    static Quat Slerp(const Quat& a, const Quat& b, float t) {
        Quat qa = a.Normalized();
        Quat qb = b.Normalized();
        
        float dot = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;
        
        // If negative dot, negate one quaternion
        if (dot < 0.0f) {
            qb.x = -qb.x;
            qb.y = -qb.y;
            qb.z = -qb.z;
            qb.w = -qb.w;
            dot = -dot;
        }
        
        // If quaternions are very close, use linear interpolation
        if (dot > 0.9995f) {
            return Quat(
                qa.x + t * (qb.x - qa.x),
                qa.y + t * (qb.y - qa.y),
                qa.z + t * (qb.z - qa.z),
                qa.w + t * (qb.w - qa.w)
            ).Normalized();
        }
        
        float theta = acosf(dot);
        float sinTheta = sinf(theta);
        float wa = sinf((1.0f - t) * theta) / sinTheta;
        float wb = sinf(t * theta) / sinTheta;
        
        return Quat(
            wa * qa.x + wb * qb.x,
            wa * qa.y + wb * qb.y,
            wa * qa.z + wb * qb.z,
            wa * qa.w + wb * qb.w
        );
    }
    
    // Convert quaternion to rotation matrix
    Mat4 ToMatrix() const {
        Mat4 result;
        
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;
        
        result.m[0] = 1.0f - 2.0f * (yy + zz);
        result.m[1] = 2.0f * (xy + wz);
        result.m[2] = 2.0f * (xz - wy);
        result.m[3] = 0.0f;
        
        result.m[4] = 2.0f * (xy - wz);
        result.m[5] = 1.0f - 2.0f * (xx + zz);
        result.m[6] = 2.0f * (yz + wx);
        result.m[7] = 0.0f;
        
        result.m[8] = 2.0f * (xz + wy);
        result.m[9] = 2.0f * (yz - wx);
        result.m[10] = 1.0f - 2.0f * (xx + yy);
        result.m[11] = 0.0f;
        
        result.m[12] = 0.0f;
        result.m[13] = 0.0f;
        result.m[14] = 0.0f;
        result.m[15] = 1.0f;
        
        return result;
    }
};

} // namespace MyEngine
