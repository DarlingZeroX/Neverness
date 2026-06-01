// ============================================================================
// Quaternion.cs - 四元数
// ============================================================================
// 四元数类型，用于表示旋转。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 四元数（旋转表示）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Quaternion : IEquatable<Quaternion>
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

    /// <summary>创建四元数。</summary>
    /// <param name="x">X 分量。</param>
    /// <param name="y">Y 分量。</param>
    /// <param name="z">Z 分量。</param>
    /// <param name="w">W 分量。</param>
    public Quaternion(float x, float y, float z, float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>单位四元数（无旋转）。</summary>
    public static readonly Quaternion Identity = new(0, 0, 0, 1);

    // ========================================================================
    // 计算属性
    // ========================================================================

    /// <summary>四元数长度。</summary>
    public readonly float Magnitude => MathF.Sqrt(SqrMagnitude);

    /// <summary>四元数长度的平方。</summary>
    public readonly float SqrMagnitude => X * X + Y * Y + Z * Z + W * W;

    /// <summary>归一化四元数。</summary>
    public readonly Quaternion Normalized
    {
        get
        {
            var mag = Magnitude;
            if (mag < 1e-6f)
                return Identity;
            return this / mag;
        }
    }

    /// <summary>共轭四元数（逆旋转）。</summary>
    public readonly Quaternion Conjugate => new(-X, -Y, -Z, W);

    /// <summary>逆四元数。</summary>
    public readonly Quaternion Inverse
    {
        get
        {
            var sqrMag = SqrMagnitude;
            if (sqrMag < 1e-6f)
                return Identity;
            return Conjugate / sqrMag;
        }
    }

    // ========================================================================
    // 方法
    // ========================================================================

    /// <summary>计算与另一四元数的点积。</summary>
    /// <param name="other">另一四元数。</param>
    /// <returns>点积值。</returns>
    public readonly float Dot(Quaternion other)
    {
        return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
    }

    /// <summary>旋转向量。</summary>
    /// <param name="point">要旋转的向量。</param>
    /// <returns>旋转后的向量。</returns>
    public readonly Vector3 RotateVector(Vector3 point)
    {
        // q * p * q^-1
        var qv = new Vector3(X, Y, Z);
        var uv = qv.Cross(point);
        var uuv = qv.Cross(uv);
        return point + (uv * W + uuv) * 2f;
    }

    // ========================================================================
    // 静态方法
    // ========================================================================

    /// <summary>从欧拉角创建四元数（度）。</summary>
    /// <param name="euler">欧拉角（度）。</param>
    /// <returns>四元数。</returns>
    public static Quaternion FromEuler(Vector3 euler)
    {
        return FromEuler(euler.X, euler.Y, euler.Z);
    }

    /// <summary>从欧拉角创建四元数（度）。</summary>
    /// <param name="x">X 轴旋转（度）。</param>
    /// <param name="y">Y 轴旋转（度）。</param>
    /// <param name="z">Z 轴旋转（度）。</param>
    /// <returns>四元数。</returns>
    public static Quaternion FromEuler(float x, float y, float z)
    {
        // 转换为弧度
        var rx = x * MathF.PI / 180f;
        var ry = y * MathF.PI / 180f;
        var rz = z * MathF.PI / 180f;

        var cx = MathF.Cos(rx * 0.5f);
        var sx = MathF.Sin(rx * 0.5f);
        var cy = MathF.Cos(ry * 0.5f);
        var sy = MathF.Sin(ry * 0.5f);
        var cz = MathF.Cos(rz * 0.5f);
        var sz = MathF.Sin(rz * 0.5f);

        return new Quaternion(
            sx * cy * cz - cx * sy * sz,
            cx * sy * cz + sx * cy * sz,
            cx * cy * sz - sx * sy * cz,
            cx * cy * cz + sx * sy * sz
        );
    }

    /// <summary>从轴角创建四元数。</summary>
    /// <param name="axis">旋转轴（必须归一化）。</param>
    /// <param name="angle">旋转角度（度）。</param>
    /// <returns>四元数。</returns>
    public static Quaternion FromAxisAngle(Vector3 axis, float angle)
    {
        var rad = angle * MathF.PI / 180f;
        var halfAngle = rad * 0.5f;
        var s = MathF.Sin(halfAngle);

        return new Quaternion(
            axis.X * s,
            axis.Y * s,
            axis.Z * s,
            MathF.Cos(halfAngle)
        );
    }

    /// <summary>从两个方向创建四元数（从 fromDirection 旋转到 toDirection）。</summary>
    /// <param name="fromDirection">起始方向。</param>
    /// <param name="toDirection">目标方向。</param>
    /// <returns>四元数。</returns>
    public static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection)
    {
        var dot = Vector3.Dot(fromDirection, toDirection);

        if (dot > 0.9999f)
        {
            return Identity;
        }

        if (dot < -0.9999f)
        {
            // 180 度旋转
            var axis = Vector3.Cross(Vector3.Right, fromDirection);
            if (axis.SqrMagnitude < 0.0001f)
            {
                axis = Vector3.Up;
            }
            return FromAxisAngle(axis.Normalized, 180f);
        }

        var axis2 = Vector3.Cross(fromDirection, toDirection);
        var w = 1f + dot;

        return new Quaternion(axis2.X, axis2.Y, axis2.Z, w).Normalized;
    }

    /// <summary>创建朝向指定方向的四元数。</summary>
    /// <param name="forward">前方向。</param>
    /// <returns>四元数。</returns>
    public static Quaternion LookRotation(Vector3 forward)
    {
        return LookRotation(forward, Vector3.Up);
    }

    /// <summary>创建朝向指定方向的四元数。</summary>
    /// <param name="forward">前方向。</param>
    /// <param name="up">上方向。</param>
    /// <returns>四元数。</returns>
    public static Quaternion LookRotation(Vector3 forward, Vector3 up)
    {
        forward = forward.Normalized;
        var right = Vector3.Cross(up, forward).Normalized;
        var newUp = Vector3.Cross(forward, right);

        // 从旋转矩阵创建四元数
        var m00 = right.X; var m01 = right.Y; var m02 = right.Z;
        var m10 = newUp.X; var m11 = newUp.Y; var m12 = newUp.Z;
        var m20 = forward.X; var m21 = forward.Y; var m22 = forward.Z;

        var trace = m00 + m11 + m22;

        if (trace > 0f)
        {
            var s = 0.5f / MathF.Sqrt(trace + 1f);
            return new Quaternion(
                (m12 - m21) * s,
                (m20 - m02) * s,
                (m01 - m10) * s,
                0.25f / s
            );
        }
        else if (m00 > m11 && m00 > m22)
        {
            var s = 2f * MathF.Sqrt(1f + m00 - m11 - m22);
            return new Quaternion(
                0.25f * s,
                (m01 + m10) / s,
                (m02 + m20) / s,
                (m12 - m21) / s
            );
        }
        else if (m11 > m22)
        {
            var s = 2f * MathF.Sqrt(1f + m11 - m00 - m22);
            return new Quaternion(
                (m01 + m10) / s,
                0.25f * s,
                (m12 + m21) / s,
                (m20 - m02) / s
            );
        }
        else
        {
            var s = 2f * MathF.Sqrt(1f + m22 - m00 - m11);
            return new Quaternion(
                (m02 + m20) / s,
                (m12 + m21) / s,
                0.25f * s,
                (m01 - m10) / s
            );
        }
    }

    /// <summary>球面线性插值。</summary>
    /// <param name="a">起始值。</param>
    /// <param name="b">目标值。</param>
    /// <param name="t">插值因子 [0, 1]。</param>
    /// <returns>插值结果。</returns>
    public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
    {
        t = Math.Clamp(t, 0f, 1f);

        var dot = a.Dot(b);

        // 如果点积为负，取反一个四元数以走最短路径
        if (dot < 0f)
        {
            b = new Quaternion(-b.X, -b.Y, -b.Z, -b.W);
            dot = -dot;
        }

        // 如果四元数非常接近，使用线性插值
        if (dot > 0.9995f)
        {
            var result = new Quaternion(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t,
                a.W + (b.W - a.W) * t
            );
            return result.Normalized;
        }

        // 球面线性插值
        var theta0 = MathF.Acos(dot);
        var theta = theta0 * t;
        var sinTheta = MathF.Sin(theta);
        var sinTheta0 = MathF.Sin(theta0);

        var s0 = MathF.Cos(theta) - dot * sinTheta / sinTheta0;
        var s1 = sinTheta / sinTheta0;

        return new Quaternion(
            a.X * s0 + b.X * s1,
            a.Y * s0 + b.Y * s1,
            a.Z * s0 + b.Z * s1,
            a.W * s0 + b.W * s1
        );
    }

    // ========================================================================
    // 运算符
    // ========================================================================

    /// <summary>四元数乘法（旋转组合）。</summary>
    public static Quaternion operator *(Quaternion a, Quaternion b) =>
        new(
            a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
            a.W * b.Y - a.X * b.Z + a.Y * b.W + a.Z * b.X,
            a.W * b.Z + a.X * b.Y - a.Y * b.X + a.Z * b.W,
            a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z
        );

    /// <summary>旋转向量。</summary>
    public static Vector3 operator *(Quaternion q, Vector3 v) =>
        q.RotateVector(v);

    /// <summary>标量乘法。</summary>
    public static Quaternion operator *(Quaternion q, float scalar) =>
        new(q.X * scalar, q.Y * scalar, q.Z * scalar, q.W * scalar);

    /// <summary>标量除法。</summary>
    public static Quaternion operator /(Quaternion q, float scalar) =>
        new(q.X / scalar, q.Y / scalar, q.Z / scalar, q.W / scalar);

    /// <summary>相等比较。</summary>
    public static bool operator ==(Quaternion a, Quaternion b) =>
        a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;

    /// <summary>不等比较。</summary>
    public static bool operator !=(Quaternion a, Quaternion b) =>
        !(a == b);

    // ========================================================================
    // IEquatable<Quaternion>
    // ========================================================================

    /// <inheritdoc/>
    public readonly bool Equals(Quaternion other) => this == other;

    /// <inheritdoc/>
    public readonly override bool Equals(object? obj) =>
        obj is Quaternion other && Equals(other);

    /// <inheritdoc/>
    public readonly override int GetHashCode() =>
        HashCode.Combine(X, Y, Z, W);

    /// <inheritdoc/>
    public readonly override string ToString() =>
        $"({X:F2}, {Y:F2}, {Z:F2}, {W:F2})";
}
