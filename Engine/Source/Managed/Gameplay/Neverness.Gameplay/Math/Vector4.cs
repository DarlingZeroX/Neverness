// ============================================================================
// Vector4.cs - 4D 向量
// ============================================================================
// 4D 向量类型。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 4D 向量。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Vector4 : IEquatable<Vector4>
{
    // ========================================================================
    // 数据字段
    // ========================================================================

    /// <summary>X 分量。</summary>
    public float X;

    /// <summary>Y 分量。</summary>
    public float Y;

    /// <summary>Z 分量。</summary>
    public float Z;

    /// <summary>W 分量。</summary>
    public float W;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建 4D 向量。</summary>
    public Vector4(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    /// <summary>创建所有分量相同的向量。</summary>
    public Vector4(float value)
    {
        X = value;
        Y = value;
        Z = value;
        W = value;
    }

    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>零向量。</summary>
    public static readonly Vector4 Zero = new(0, 0, 0, 0);

    /// <summary>单位向量。</summary>
    public static readonly Vector4 One = new(1, 1, 1, 1);

    // ========================================================================
    // 运算符
    // ========================================================================

    /// <summary>向量加法。</summary>
    public static Vector4 operator +(Vector4 a, Vector4 b) =>
        new(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);

    /// <summary>向量减法。</summary>
    public static Vector4 operator -(Vector4 a, Vector4 b) =>
        new(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);

    /// <summary>标量乘法。</summary>
    public static Vector4 operator *(Vector4 v, float scalar) =>
        new(v.X * scalar, v.Y * scalar, v.Z * scalar, v.W * scalar);

    /// <summary>标量除法。</summary>
    public static Vector4 operator /(Vector4 v, float scalar) =>
        new(v.X / scalar, v.Y / scalar, v.Z / scalar, v.W / scalar);

    /// <summary>相等比较。</summary>
    public static bool operator ==(Vector4 a, Vector4 b) =>
        a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;

    /// <summary>不等比较。</summary>
    public static bool operator !=(Vector4 a, Vector4 b) =>
        !(a == b);

    // ========================================================================
    // IEquatable<Vector4>
    // ========================================================================

    /// <inheritdoc/>
    public readonly bool Equals(Vector4 other) => this == other;

    /// <inheritdoc/>
    public readonly override bool Equals(object? obj) =>
        obj is Vector4 other && Equals(other);

    /// <inheritdoc/>
    public readonly override int GetHashCode() =>
        HashCode.Combine(X, Y, Z, W);

    /// <inheritdoc/>
    public readonly override string ToString() =>
        $"({X:F2}, {Y:F2}, {Z:F2}, {W:F2})";
}
