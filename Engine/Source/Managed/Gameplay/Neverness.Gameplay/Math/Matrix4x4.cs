// ============================================================================
// Matrix4x4.cs - 4x4 矩阵
// ============================================================================
// 4x4 矩阵类型，用于变换和投影。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 4x4 矩阵（列主序）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Matrix4x4 : IEquatable<Matrix4x4>
{
    // ========================================================================
    // 数据字段（列主序存储）
    // ========================================================================

    /// <summary>第一列。</summary>
    public Vector4 Column0;

    /// <summary>第二列。</summary>
    public Vector4 Column1;

    /// <summary>第三列。</summary>
    public Vector4 Column2;

    /// <summary>第四列。</summary>
    public Vector4 Column3;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建 4x4 矩阵。</summary>
    public Matrix4x4(Vector4 col0, Vector4 col1, Vector4 col2, Vector4 col3)
    {
        Column0 = col0;
        Column1 = col1;
        Column2 = col2;
        Column3 = col3;
    }

    // ========================================================================
    // 静态属性
    // ========================================================================

    /// <summary>单位矩阵。</summary>
    public static readonly Matrix4x4 Identity = new(
        new Vector4(1, 0, 0, 0),
        new Vector4(0, 1, 0, 0),
        new Vector4(0, 0, 1, 0),
        new Vector4(0, 0, 0, 1)
    );

    // ========================================================================
    // 索引器
    // ========================================================================

    /// <summary>按列、行索引访问元素。</summary>
    public float this[int column, int row]
    {
        readonly get => column switch
        {
            0 => row switch { 0 => Column0.X, 1 => Column0.Y, 2 => Column0.Z, 3 => Column0.W, _ => throw new IndexOutOfRangeException() },
            1 => row switch { 0 => Column1.X, 1 => Column1.Y, 2 => Column1.Z, 3 => Column1.W, _ => throw new IndexOutOfRangeException() },
            2 => row switch { 0 => Column2.X, 1 => Column2.Y, 2 => Column2.Z, 3 => Column2.W, _ => throw new IndexOutOfRangeException() },
            3 => row switch { 0 => Column3.X, 1 => Column3.Y, 2 => Column3.Z, 3 => Column3.W, _ => throw new IndexOutOfRangeException() },
            _ => throw new IndexOutOfRangeException()
        };
        set
        {
            switch (column)
            {
                case 0: switch (row) { case 0: Column0.X = value; break; case 1: Column0.Y = value; break; case 2: Column0.Z = value; break; case 3: Column0.W = value; break; default: throw new IndexOutOfRangeException(); } break;
                case 1: switch (row) { case 0: Column1.X = value; break; case 1: Column1.Y = value; break; case 2: Column1.Z = value; break; case 3: Column1.W = value; break; default: throw new IndexOutOfRangeException(); } break;
                case 2: switch (row) { case 0: Column2.X = value; break; case 1: Column2.Y = value; break; case 2: Column2.Z = value; break; case 3: Column2.W = value; break; default: throw new IndexOutOfRangeException(); } break;
                case 3: switch (row) { case 0: Column3.X = value; break; case 1: Column3.Y = value; break; case 2: Column3.Z = value; break; case 3: Column3.W = value; break; default: throw new IndexOutOfRangeException(); } break;
                default: throw new IndexOutOfRangeException();
            }
        }
    }

    // ========================================================================
    // 静态方法
    // ========================================================================

    /// <summary>创建平移矩阵。</summary>
    /// <param name="translation">平移向量。</param>
    /// <returns>平移矩阵。</returns>
    public static Matrix4x4 CreateTranslation(Vector3 translation)
    {
        return new Matrix4x4(
            new Vector4(1, 0, 0, 0),
            new Vector4(0, 1, 0, 0),
            new Vector4(0, 0, 1, 0),
            new Vector4(translation.X, translation.Y, translation.Z, 1)
        );
    }

    /// <summary>创建缩放矩阵。</summary>
    /// <param name="scale">缩放向量。</param>
    /// <returns>缩放矩阵。</returns>
    public static Matrix4x4 CreateScale(Vector3 scale)
    {
        return new Matrix4x4(
            new Vector4(scale.X, 0, 0, 0),
            new Vector4(0, scale.Y, 0, 0),
            new Vector4(0, 0, scale.Z, 0),
            new Vector4(0, 0, 0, 1)
        );
    }

    /// <summary>创建旋转矩阵（从四元数）。</summary>
    /// <param name="rotation">旋转四元数。</param>
    /// <returns>旋转矩阵。</returns>
    public static Matrix4x4 CreateFromQuaternion(Quaternion rotation)
    {
        var x = rotation.X;
        var y = rotation.Y;
        var z = rotation.Z;
        var w = rotation.W;

        var x2 = x + x;
        var y2 = y + y;
        var z2 = z + z;

        var xx = x * x2;
        var xy = x * y2;
        var xz = x * z2;
        var yy = y * y2;
        var yz = y * z2;
        var zz = z * z2;
        var wx = w * x2;
        var wy = w * y2;
        var wz = w * z2;

        return new Matrix4x4(
            new Vector4(1 - (yy + zz), xy + wz, xz - wy, 0),
            new Vector4(xy - wz, 1 - (xx + zz), yz + wx, 0),
            new Vector4(xz + wy, yz - wx, 1 - (xx + yy), 0),
            new Vector4(0, 0, 0, 1)
        );
    }

    /// <summary>创建 TRS 矩阵（平移-旋转-缩放）。</summary>
    /// <param name="position">位置。</param>
    /// <param name="rotation">旋转。</param>
    /// <param name="scale">缩放。</param>
    /// <returns>TRS 矩阵。</returns>
    public static Matrix4x4 CreateTRS(Vector3 position, Quaternion rotation, Vector3 scale)
    {
        var r = CreateFromQuaternion(rotation);
        var s = CreateScale(scale);
        var t = CreateTranslation(position);

        return t * r * s;
    }

    /// <summary>创建透视投影矩阵。</summary>
    /// <param name="fieldOfView">视场角（弧度）。</param>
    /// <param name="aspectRatio">宽高比。</param>
    /// <param name="nearPlane">近裁剪面。</param>
    /// <param name="farPlane">远裁剪面。</param>
    /// <returns>透视投影矩阵。</returns>
    public static Matrix4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
    {
        var f = 1.0f / MathF.Tan(fieldOfView * 0.5f);
        var rangeInv = 1.0f / (nearPlane - farPlane);

        return new Matrix4x4(
            new Vector4(f / aspectRatio, 0, 0, 0),
            new Vector4(0, f, 0, 0),
            new Vector4(0, 0, (farPlane + nearPlane) * rangeInv, -1),
            new Vector4(0, 0, 2 * farPlane * nearPlane * rangeInv, 0)
        );
    }

    /// <summary>创建正交投影矩阵。</summary>
    /// <param name="width">宽度。</param>
    /// <param name="height">高度。</param>
    /// <param name="nearPlane">近裁剪面。</param>
    /// <param name="farPlane">远裁剪面。</param>
    /// <returns>正交投影矩阵。</returns>
    public static Matrix4x4 CreateOrthographic(float width, float height, float nearPlane, float farPlane)
    {
        var rangeInv = 1.0f / (nearPlane - farPlane);

        return new Matrix4x4(
            new Vector4(2.0f / width, 0, 0, 0),
            new Vector4(0, 2.0f / height, 0, 0),
            new Vector4(0, 0, rangeInv, 0),
            new Vector4(0, 0, nearPlane * rangeInv, 1)
        );
    }

    /// <summary>创建视图矩阵（LookAt）。</summary>
    /// <param name="eye">摄像机位置。</param>
    /// <param name="target">目标位置。</param>
    /// <param name="up">上方向。</param>
    /// <returns>视图矩阵。</returns>
    public static Matrix4x4 CreateLookAt(Vector3 eye, Vector3 target, Vector3 up)
    {
        var z = (eye - target).Normalized;
        var x = Vector3.Cross(up, z).Normalized;
        var y = Vector3.Cross(z, x);

        return new Matrix4x4(
            new Vector4(x.X, y.X, z.X, 0),
            new Vector4(x.Y, y.Y, z.Y, 0),
            new Vector4(x.Z, y.Z, z.Z, 0),
            new Vector4(-Vector3.Dot(x, eye), -Vector3.Dot(y, eye), -Vector3.Dot(z, eye), 1)
        );
    }

    /// <summary>计算矩阵的逆。</summary>
    /// <returns>逆矩阵。</returns>
    public readonly Matrix4x4 Inverse()
    {
        var m = this;
        var inv = new float[16];
        var det = 0f;

        inv[0] = m[1, 1] * m[2, 2] * m[3, 3] - m[1, 1] * m[2, 3] * m[3, 2] - m[2, 1] * m[1, 2] * m[3, 3] + m[2, 1] * m[1, 3] * m[3, 2] + m[3, 1] * m[1, 2] * m[2, 3] - m[3, 1] * m[1, 3] * m[2, 2];
        inv[4] = -m[0, 1] * m[2, 2] * m[3, 3] + m[0, 1] * m[2, 3] * m[3, 2] + m[2, 1] * m[0, 2] * m[3, 3] - m[2, 1] * m[0, 3] * m[3, 2] - m[3, 1] * m[0, 2] * m[2, 3] + m[3, 1] * m[0, 3] * m[2, 2];
        inv[8] = m[0, 1] * m[1, 2] * m[3, 3] - m[0, 1] * m[1, 3] * m[3, 2] - m[1, 1] * m[0, 2] * m[3, 3] + m[1, 1] * m[0, 3] * m[3, 2] + m[3, 1] * m[0, 2] * m[1, 3] - m[3, 1] * m[0, 3] * m[1, 2];
        inv[12] = -m[0, 1] * m[1, 2] * m[2, 3] + m[0, 1] * m[1, 3] * m[2, 2] + m[1, 1] * m[0, 2] * m[2, 3] - m[1, 1] * m[0, 3] * m[2, 2] - m[2, 1] * m[0, 2] * m[1, 3] + m[2, 1] * m[0, 3] * m[1, 2];

        det = m[0, 0] * inv[0] + m[1, 0] * inv[4] + m[2, 0] * inv[8] + m[3, 0] * inv[12];

        if (MathF.Abs(det) < 1e-10f)
            return Identity;

        var invDet = 1.0f / det;

        return new Matrix4x4(
            new Vector4(inv[0] * invDet, inv[1] * invDet, inv[2] * invDet, inv[3] * invDet),
            new Vector4(inv[4] * invDet, inv[5] * invDet, inv[6] * invDet, inv[7] * invDet),
            new Vector4(inv[8] * invDet, inv[9] * invDet, inv[10] * invDet, inv[11] * invDet),
            new Vector4(inv[12] * invDet, inv[13] * invDet, inv[14] * invDet, inv[15] * invDet)
        );
    }

    /// <summary>变换点（应用完整的 TRS 变换）。</summary>
    /// <param name="point">要变换的点。</param>
    /// <returns>变换后的点。</returns>
    public readonly Vector3 TransformPoint(Vector3 point)
    {
        var w = Column0.W * point.X + Column1.W * point.Y + Column2.W * point.Z + Column3.W;
        return new Vector3(
            (Column0.X * point.X + Column1.X * point.Y + Column2.X * point.Z + Column3.X) / w,
            (Column0.Y * point.X + Column1.Y * point.Y + Column2.Y * point.Z + Column3.Y) / w,
            (Column0.Z * point.X + Column1.Z * point.Y + Column2.Z * point.Z + Column3.Z) / w
        );
    }

    /// <summary>变换方向（不应用平移）。</summary>
    /// <param name="direction">要变换的方向。</param>
    /// <returns>变换后的方向。</returns>
    public readonly Vector3 TransformDirection(Vector3 direction)
    {
        return new Vector3(
            Column0.X * direction.X + Column1.X * direction.Y + Column2.X * direction.Z,
            Column0.Y * direction.X + Column1.Y * direction.Y + Column2.Y * direction.Z,
            Column0.Z * direction.X + Column1.Z * direction.Y + Column2.Z * direction.Z
        );
    }

    // ========================================================================
    // 运算符
    // ========================================================================

    /// <summary>矩阵乘法。</summary>
    public static Matrix4x4 operator *(Matrix4x4 a, Matrix4x4 b)
    {
        return new Matrix4x4(
            new Vector4(
                a.Column0.X * b.Column0.X + a.Column1.X * b.Column0.Y + a.Column2.X * b.Column0.Z + a.Column3.X * b.Column0.W,
                a.Column0.Y * b.Column0.X + a.Column1.Y * b.Column0.Y + a.Column2.Y * b.Column0.Z + a.Column3.Y * b.Column0.W,
                a.Column0.Z * b.Column0.X + a.Column1.Z * b.Column0.Y + a.Column2.Z * b.Column0.Z + a.Column3.Z * b.Column0.W,
                a.Column0.W * b.Column0.X + a.Column1.W * b.Column0.Y + a.Column2.W * b.Column0.Z + a.Column3.W * b.Column0.W
            ),
            new Vector4(
                a.Column0.X * b.Column1.X + a.Column1.X * b.Column1.Y + a.Column2.X * b.Column1.Z + a.Column3.X * b.Column1.W,
                a.Column0.Y * b.Column1.X + a.Column1.Y * b.Column1.Y + a.Column2.Y * b.Column1.Z + a.Column3.Y * b.Column1.W,
                a.Column0.Z * b.Column1.X + a.Column1.Z * b.Column1.Y + a.Column2.Z * b.Column1.Z + a.Column3.Z * b.Column1.W,
                a.Column0.W * b.Column1.X + a.Column1.W * b.Column1.Y + a.Column2.W * b.Column1.Z + a.Column3.W * b.Column1.W
            ),
            new Vector4(
                a.Column0.X * b.Column2.X + a.Column1.X * b.Column2.Y + a.Column2.X * b.Column2.Z + a.Column3.X * b.Column2.W,
                a.Column0.Y * b.Column2.X + a.Column1.Y * b.Column2.Y + a.Column2.Y * b.Column2.Z + a.Column3.Y * b.Column2.W,
                a.Column0.Z * b.Column2.X + a.Column1.Z * b.Column2.Y + a.Column2.Z * b.Column2.Z + a.Column3.Z * b.Column2.W,
                a.Column0.W * b.Column2.X + a.Column1.W * b.Column2.Y + a.Column2.W * b.Column2.Z + a.Column3.W * b.Column2.W
            ),
            new Vector4(
                a.Column0.X * b.Column3.X + a.Column1.X * b.Column3.Y + a.Column2.X * b.Column3.Z + a.Column3.X * b.Column3.W,
                a.Column0.Y * b.Column3.X + a.Column1.Y * b.Column3.Y + a.Column2.Y * b.Column3.Z + a.Column3.Y * b.Column3.W,
                a.Column0.Z * b.Column3.X + a.Column1.Z * b.Column3.Y + a.Column2.Z * b.Column3.Z + a.Column3.Z * b.Column3.W,
                a.Column0.W * b.Column3.X + a.Column1.W * b.Column3.Y + a.Column2.W * b.Column3.Z + a.Column3.W * b.Column3.W
            )
        );
    }

    /// <summary>相等比较。</summary>
    public static bool operator ==(Matrix4x4 a, Matrix4x4 b) =>
        a.Column0 == b.Column0 && a.Column1 == b.Column1 && a.Column2 == b.Column2 && a.Column3 == b.Column3;

    /// <summary>不等比较。</summary>
    public static bool operator !=(Matrix4x4 a, Matrix4x4 b) => !(a == b);

    // ========================================================================
    // IEquatable<Matrix4x4>
    // ========================================================================

    /// <inheritdoc/>
    public readonly bool Equals(Matrix4x4 other) => this == other;

    /// <inheritdoc/>
    public readonly override bool Equals(object? obj) =>
        obj is Matrix4x4 other && Equals(other);

    /// <inheritdoc/>
    public readonly override int GetHashCode() =>
        HashCode.Combine(Column0, Column1, Column2, Column3);

    /// <inheritdoc/>
    public readonly override string ToString() =>
        $"[{Column0}, {Column1}, {Column2}, {Column3}]";
}
