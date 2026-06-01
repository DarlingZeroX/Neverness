// ============================================================================
// Vector3.cs - 3D 向量
// ============================================================================
// 3D 向量类型，与 Native 数学库内存布局对齐。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 3D 向量。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Vector3 : IEquatable<Vector3>
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

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建 3D 向量。</summary>
    /// <param name="x">X 分量。</param>
    /// <param name="y">Y 分量。</param>
    /// <param name="z">Z 分量。</param>
    public Vector3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    /// <summary>创建所有分量相同的向量。</summary>
    /// <param name="value">分量值。</param>
    public Vector3(float value)
    {
        X = value;
        Y = value;
        Z = value;
    }

    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>零向量 (0, 0, 0)。</summary>
    public static readonly Vector3 Zero = new(0, 0, 0);

    /// <summary>单位向量 (1, 1, 1)。</summary>
    public static readonly Vector3 One = new(1, 1, 1);

    /// <summary>右方向 (1, 0, 0)。</summary>
    public static readonly Vector3 Right = new(1, 0, 0);

    /// <summary>左方向 (-1, 0, 0)。</summary>
    public static readonly Vector3 Left = new(-1, 0, 0);

    /// <summary>上方向 (0, 1, 0)。</summary>
    public static readonly Vector3 Up = new(0, 1, 0);

    /// <summary>下方向 (0, -1, 0)。</summary>
    public static readonly Vector3 Down = new(0, -1, 0);

    /// <summary>前方向 (0, 0, -1)（右手坐标系）。</summary>
    public static readonly Vector3 Forward = new(0, 0, -1);

    /// <summary>后方向 (0, 0, 1)（右手坐标系）。</summary>
    public static readonly Vector3 Back = new(0, 0, 1);

    // ========================================================================
    // 计算属性
    // ========================================================================

    /// <summary>向量长度。</summary>
    public readonly float Magnitude => MathF.Sqrt(SqrMagnitude);

    /// <summary>向量长度的平方（避免开方，用于比较）。</summary>
    public readonly float SqrMagnitude => X * X + Y * Y + Z * Z;

    /// <summary>归一化向量。</summary>
    public readonly Vector3 Normalized
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
    public readonly float Dot(Vector3 other)
    {
        return X * other.X + Y * other.Y + Z * other.Z;
    }

    /// <summary>计算与另一向量的叉积。</summary>
    /// <param name="other">另一向量。</param>
    /// <returns>叉积向量。</returns>
    public readonly Vector3 Cross(Vector3 other)
    {
        return new Vector3(
            Y * other.Z - Z * other.Y,
            Z * other.X - X * other.Z,
            X * other.Y - Y * other.X
        );
    }

    /// <summary>计算与另一向量的距离。</summary>
    /// <param name="other">另一向量。</param>
    /// <returns>距离值。</returns>
    public readonly float Distance(Vector3 other)
    {
        return (this - other).Magnitude;
    }

    /// <summary>计算与另一向量的距离的平方。</summary>
    /// <param name="other">另一向量。</param>
    /// <returns>距离的平方。</returns>
    public readonly float SqrDistance(Vector3 other)
    {
        return (this - other).SqrMagnitude;
    }

    // ========================================================================
    // 静态方法
    // ========================================================================

    /// <summary>计算两个向量的点积。</summary>
    /// <param name="a">向量 A。</param>
    /// <param name="b">向量 B。</param>
    /// <returns>点积值。</returns>
    public static float Dot(Vector3 a, Vector3 b) => a.Dot(b);

    /// <summary>计算两个向量的叉积。</summary>
    /// <param name="a">向量 A。</param>
    /// <param name="b">向量 B。</param>
    /// <returns>叉积向量。</returns>
    public static Vector3 Cross(Vector3 a, Vector3 b) => a.Cross(b);

    /// <summary>计算两个向量的距离。</summary>
    /// <param name="a">向量 A。</param>
    /// <param name="b">向量 B。</param>
    /// <returns>距离值。</returns>
    public static float Distance(Vector3 a, Vector3 b) => a.Distance(b);

    /// <summary>线性插值。</summary>
    /// <param name="a">起始值。</param>
    /// <param name="b">目标值。</param>
    /// <param name="t">插值因子 [0, 1]。</param>
    /// <returns>插值结果。</returns>
    public static Vector3 Lerp(Vector3 a, Vector3 b, float t)
    {
        t = Math.Clamp(t, 0f, 1f);
        return new Vector3(
            a.X + (b.X - a.X) * t,
            a.Y + (b.Y - a.Y) * t,
            a.Z + (b.Z - a.Z) * t
        );
    }

    /// <summary>限制向量长度不超过最大值。</summary>
    /// <param name="vector">输入向量。</param>
    /// <param name="maxLength">最大长度。</param>
    /// <returns>限制后的向量。</returns>
    public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
    {
        var sqrMag = vector.SqrMagnitude;
        if (sqrMag > maxLength * maxLength)
        {
            return vector.Normalized * maxLength;
        }
        return vector;
    }

    // ========================================================================
    // 运算符
    // ========================================================================

    /// <summary>向量加法。</summary>
    public static Vector3 operator +(Vector3 a, Vector3 b) =>
        new(a.X + b.X, a.Y + b.Y, a.Z + b.Z);

    /// <summary>向量减法。</summary>
    public static Vector3 operator -(Vector3 a, Vector3 b) =>
        new(a.X - b.X, a.Y - b.Y, a.Z - b.Z);

    /// <summary>向量取反。</summary>
    public static Vector3 operator -(Vector3 v) =>
        new(-v.X, -v.Y, -v.Z);

    /// <summary>标量乘法。</summary>
    public static Vector3 operator *(Vector3 v, float scalar) =>
        new(v.X * scalar, v.Y * scalar, v.Z * scalar);

    /// <summary>标量乘法。</summary>
    public static Vector3 operator *(float scalar, Vector3 v) =>
        new(v.X * scalar, v.Y * scalar, v.Z * scalar);

    /// <summary>标量除法。</summary>
    public static Vector3 operator /(Vector3 v, float scalar) =>
        new(v.X / scalar, v.Y / scalar, v.Z / scalar);

    /// <summary>相等比较。</summary>
    public static bool operator ==(Vector3 a, Vector3 b) =>
        a.X == b.X && a.Y == b.Y && a.Z == b.Z;

    /// <summary>不等比较。</summary>
    public static bool operator !=(Vector3 a, Vector3 b) =>
        !(a == b);

    // ========================================================================
    // IEquatable<Vector3>
    // ========================================================================

    /// <inheritdoc/>
    public readonly bool Equals(Vector3 other) => this == other;

    /// <inheritdoc/>
    public readonly override bool Equals(object? obj) =>
        obj is Vector3 other && Equals(other);

    /// <inheritdoc/>
    public readonly override int GetHashCode() =>
        HashCode.Combine(X, Y, Z);

    /// <inheritdoc/>
    public readonly override string ToString() =>
        $"({X:F2}, {Y:F2}, {Z:F2})";
}
