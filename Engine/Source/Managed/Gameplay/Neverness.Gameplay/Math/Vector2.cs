// ============================================================================
// Vector2.cs - 2D 向量
// ============================================================================
// 2D 向量类型。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 2D 向量。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Vector2 : IEquatable<Vector2>
{
    // ========================================================================
    // 数据字段
    // ========================================================================

    /// <summary>X 分量。</summary>
    public float X;

    /// <summary>Y 分量。</summary>
    public float Y;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建 2D 向量。</summary>
    /// <param name="x">X 分量。</param>
    /// <param name="y">Y 分量。</param>
    public Vector2(float x, float y)
    {
        X = x;
        Y = y;
    }

    /// <summary>创建所有分量相同的向量。</summary>
    /// <param name="value">分量值。</param>
    public Vector2(float value)
    {
        X = value;
        Y = value;
    }

    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>零向量 (0, 0)。</summary>
    public static readonly Vector2 Zero = new(0, 0);

    /// <summary>单位向量 (1, 1)。</summary>
    public static readonly Vector2 One = new(1, 1);

    /// <summary>右方向 (1, 0)。</summary>
    public static readonly Vector2 Right = new(1, 0);

    /// <summary>左方向 (-1, 0)。</summary>
    public static readonly Vector2 Left = new(-1, 0);

    /// <summary>上方向 (0, 1)。</summary>
    public static readonly Vector2 Up = new(0, 1);

    /// <summary>下方向 (0, -1)。</summary>
    public static readonly Vector2 Down = new(0, -1);

    // ========================================================================
    // 计算属性
    // ========================================================================

    /// <summary>向量长度。</summary>
    public readonly float Magnitude => MathF.Sqrt(SqrMagnitude);

    /// <summary>向量长度的平方。</summary>
    public readonly float SqrMagnitude => X * X + Y * Y;

    /// <summary>归一化向量。</summary>
    public readonly Vector2 Normalized
    {
        get
        {
            var mag = Magnitude;
            if (mag < 1e-6f)
                return Zero;
            return this / mag;
        }
    }

    // ========================================================================
    // 方法
    // ========================================================================

    /// <summary>计算与另一向量的点积。</summary>
    /// <param name="other">另一向量。</param>
    /// <returns>点积值。</returns>
    public readonly float Dot(Vector2 other)
    {
        return X * other.X + Y * other.Y;
    }

    /// <summary>计算与另一向量的距离。</summary>
    /// <param name="other">另一向量。</param>
    /// <returns>距离值。</returns>
    public readonly float Distance(Vector2 other)
    {
        return (this - other).Magnitude;
    }

    // ========================================================================
    // 静态方法
    // ========================================================================

    /// <summary>线性插值。</summary>
    /// <param name="a">起始值。</param>
    /// <param name="b">目标值。</param>
    /// <param name="t">插值因子 [0, 1]。</param>
    /// <returns>插值结果。</returns>
    public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
    {
        t = Math.Clamp(t, 0f, 1f);
        return new Vector2(
            a.X + (b.X - a.X) * t,
            a.Y + (b.Y - a.Y) * t
        );
    }

    // ========================================================================
    // 运算符
    // ========================================================================

    /// <summary>向量加法。</summary>
    public static Vector2 operator +(Vector2 a, Vector2 b) =>
        new(a.X + b.X, a.Y + b.Y);

    /// <summary>向量减法。</summary>
    public static Vector2 operator -(Vector2 a, Vector2 b) =>
        new(a.X - b.X, a.Y - b.Y);

    /// <summary>向量取反。</summary>
    public static Vector2 operator -(Vector2 v) =>
        new(-v.X, -v.Y);

    /// <summary>标量乘法。</summary>
    public static Vector2 operator *(Vector2 v, float scalar) =>
        new(v.X * scalar, v.Y * scalar);

    /// <summary>标量乘法。</summary>
    public static Vector2 operator *(float scalar, Vector2 v) =>
        new(v.X * scalar, v.Y * scalar);

    /// <summary>标量除法。</summary>
    public static Vector2 operator /(Vector2 v, float scalar) =>
        new(v.X / scalar, v.Y / scalar);

    /// <summary>相等比较。</summary>
    public static bool operator ==(Vector2 a, Vector2 b) =>
        a.X == b.X && a.Y == b.Y;

    /// <summary>不等比较。</summary>
    public static bool operator !=(Vector2 a, Vector2 b) =>
        !(a == b);

    // ========================================================================
    // IEquatable<Vector2>
    // ========================================================================

    /// <inheritdoc/>
    public readonly bool Equals(Vector2 other) => this == other;

    /// <inheritdoc/>
    public readonly override bool Equals(object? obj) =>
        obj is Vector2 other && Equals(other);

    /// <inheritdoc/>
    public readonly override int GetHashCode() =>
        HashCode.Combine(X, Y);

    /// <inheritdoc/>
    public readonly override string ToString() =>
        $"({X:F2}, {Y:F2})";
}
