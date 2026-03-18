#pragma once

#include <cmath>

namespace Math
{
    constexpr float Pi = 3.14159265358979323846f;

    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        Vec2(float x_, float y_) : x(x_), y(y_) {}

        Vec2 operator+(const Vec2& other) const
        {
            return Vec2(x + other.x, y + other.y);
        }

        Vec2 operator-(const Vec2& other) const
        {
            return Vec2(x - other.x, y - other.y);
        }

        Vec2 operator*(float scalar) const
        {
            return Vec2(x * scalar, y * scalar);
        }

        Vec2 operator/(float scalar) const
        {
            return scalar != 0.0f ? Vec2(x / scalar, y / scalar) : Vec2();
        }

        Vec2& operator+=(const Vec2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2& operator-=(const Vec2& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2& operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        Vec2& operator/=(float scalar)
        {
            if (scalar != 0.0f)
            {
                x /= scalar;
                y /= scalar;
            }
            else
            {
                x = 0.0f;
                y = 0.0f;
            }
            return *this;
        }

        float LengthSq() const
        {
            return x * x + y * y;
        }

        float Length() const
        {
            return std::sqrt(LengthSq());
        }

        Vec2 Normalized() const
        {
            const float len = Length();
            if (len <= 1e-6f)
                return Vec2(0.0f, 0.0f);
            return Vec2(x / len, y / len);
        }

        static float Dot(const Vec2& a, const Vec2& b)
        {
            return a.x * b.x + a.y * b.y;
        }
    };

    inline Vec2 operator*(float scalar, const Vec2& v)
    {
        return Vec2(v.x * scalar, v.y * scalar);
    }

    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vec3() = default;
        Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

        Vec3 operator+(const Vec3& other) const
        {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other) const
        {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(float scalar) const
        {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator/(float scalar) const
        {
            return scalar != 0.0f ? Vec3(x / scalar, y / scalar, z / scalar) : Vec3();
        }

        Vec3& operator+=(const Vec3& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vec3& operator-=(const Vec3& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vec3& operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vec3& operator/=(float scalar)
        {
            if (scalar != 0.0f)
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
            }
            else
            {
                x = 0.0f;
                y = 0.0f;
                z = 0.0f;
            }
            return *this;
        }

        float LengthSq() const
        {
            return x * x + y * y + z * z;
        }

        float Length() const
        {
            return std::sqrt(LengthSq());
        }

        Vec3 Normalized() const
        {
            const float len = Length();
            if (len <= 1e-6f)
                return Vec3(0.0f, 0.0f, 0.0f);
            return Vec3(x / len, y / len, z / len);
        }

        static float Dot(const Vec3& a, const Vec3& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        static Vec3 Cross(const Vec3& a, const Vec3& b)
        {
            return Vec3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }
    };

    inline Vec3 operator*(float scalar, const Vec3& v)
    {
        return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    inline float ToRadians(float degrees)
    {
        return degrees * (Pi / 180.0f);
    }

    inline float ToDegrees(float radians)
    {
        return radians * (180.0f / Pi);
    }

    inline Vec3 RotateVecX(const Vec3& v, float angle)
    {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        return Vec3(
            v.x,
            v.y * c - v.z * s,
            v.y * s + v.z * c
        );
    }

    inline Vec3 RotateVecY(const Vec3& v, float angle)
    {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        return Vec3(
            v.x * c + v.z * s,
            v.y,
            -v.x * s + v.z * c
        );
    }

    inline Vec3 RotateVecZ(const Vec3& v, float angle)
    {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        return Vec3(
            v.x * c - v.y * s,
            v.x * s + v.y * c,
            v.z
        );
    }

    inline Vec3 RotateVecXYZ(const Vec3& v, float rx, float ry, float rz)
    {
        return RotateVecZ(RotateVecY(RotateVecX(v, rx), ry), rz);
    }
}