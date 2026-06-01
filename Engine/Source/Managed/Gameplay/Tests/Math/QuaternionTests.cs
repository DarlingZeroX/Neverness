// ============================================================================
// QuaternionTests.cs - Quaternion 单元测试
// ============================================================================

using Neverness.Gameplay;
using Xunit;

namespace Neverness.Gameplay.Tests.Math;

/// <summary>
/// Quaternion 单元测试。
/// </summary>
public class QuaternionTests
{
    // ========================================================================
    // 构造函数测试
    // ========================================================================

    [Fact]
    public void Constructor_SetsComponents()
    {
        var q = new Quaternion(1f, 2f, 3f, 4f);

        Assert.Equal(1f, q.X);
        Assert.Equal(2f, q.Y);
        Assert.Equal(3f, q.Z);
        Assert.Equal(4f, q.W);
    }

    // ========================================================================
    // 静态属性测试
    // ========================================================================

    [Fact]
    public void Identity_ReturnsIdentityQuaternion()
    {
        var q = Quaternion.Identity;

        Assert.Equal(0f, q.X);
        Assert.Equal(0f, q.Y);
        Assert.Equal(0f, q.Z);
        Assert.Equal(1f, q.W);
    }

    // ========================================================================
    // 计算属性测试
    // ========================================================================

    [Fact]
    public void Magnitude_ReturnsCorrectLength()
    {
        var q = new Quaternion(1f, 2f, 3f, 4f);

        Assert.Equal(MathF.Sqrt(30f), q.Magnitude, 4);
    }

    [Fact]
    public void SqrMagnitude_ReturnsCorrectSquaredLength()
    {
        var q = new Quaternion(1f, 2f, 3f, 4f);

        Assert.Equal(30f, q.SqrMagnitude, 4);
    }

    [Fact]
    public void Normalized_ReturnsUnitQuaternion()
    {
        var q = new Quaternion(1f, 2f, 3f, 4f);
        var n = q.Normalized;

        Assert.Equal(1f, n.Magnitude, 4);
    }

    [Fact]
    public void Conjugate_ReturnsCorrectValue()
    {
        var q = new Quaternion(1f, 2f, 3f, 4f);
        var c = q.Conjugate;

        Assert.Equal(-1f, c.X);
        Assert.Equal(-2f, c.Y);
        Assert.Equal(-3f, c.Z);
        Assert.Equal(4f, c.W);
    }

    [Fact]
    public void Inverse_ReturnsCorrectValue()
    {
        var q = Quaternion.Identity;
        var inv = q.Inverse;

        Assert.Equal(0f, inv.X, 4);
        Assert.Equal(0f, inv.Y, 4);
        Assert.Equal(0f, inv.Z, 4);
        Assert.Equal(1f, inv.W, 4);
    }

    // ========================================================================
    // 方法测试
    // ========================================================================

    [Fact]
    public void Dot_ReturnsCorrectValue()
    {
        var a = new Quaternion(1f, 2f, 3f, 4f);
        var b = new Quaternion(5f, 6f, 7f, 8f);

        var dot = a.Dot(b);

        Assert.Equal(70f, dot, 4);
    }

    [Fact]
    public void RotateVector_Identity_DoesNotChangeVector()
    {
        var q = Quaternion.Identity;
        var v = new Vector3(1f, 2f, 3f);

        var result = q.RotateVector(v);

        Assert.Equal(1f, result.X, 4);
        Assert.Equal(2f, result.Y, 4);
        Assert.Equal(3f, result.Z, 4);
    }

    [Fact]
    public void FromEuler_0Degrees_ReturnsIdentity()
    {
        var q = Quaternion.FromEuler(0f, 0f, 0f);

        Assert.Equal(0f, q.X, 4);
        Assert.Equal(0f, q.Y, 4);
        Assert.Equal(0f, q.Z, 4);
        Assert.Equal(1f, q.W, 4);
    }

    [Fact]
    public void FromAxisAngle_90DegreesAroundY_RotatesCorrectly()
    {
        var q = Quaternion.FromAxisAngle(Vector3.Up, 90f);
        var v = Vector3.Right;

        var result = q.RotateVector(v);

        Assert.Equal(0f, result.X, 4);
        Assert.Equal(0f, result.Y, 4);
        Assert.Equal(-1f, result.Z, 4);
    }

    [Fact]
    public void Slerp_InterpolatesCorrectly()
    {
        var a = Quaternion.Identity;
        var b = Quaternion.FromAxisAngle(Vector3.Up, 90f);

        var result = Quaternion.Slerp(a, b, 0.5f);

        // 45度旋转
        var v = Vector3.Right;
        var rotated = result.RotateVector(v);

        Assert.Equal(0.7071f, rotated.X, 3);
        Assert.Equal(0f, rotated.Y, 4);
        Assert.Equal(-0.7071f, rotated.Z, 3);
    }

    [Fact]
    public void LookRotation_ForwardDirection_ReturnsCorrectRotation()
    {
        var forward = Vector3.Forward;
        var q = Quaternion.LookRotation(forward);

        // 朝向 -Z 方向应该是单位四元数（或接近）
        var v = q.RotateVector(Vector3.Forward);

        Assert.Equal(0f, v.X, 3);
        Assert.Equal(0f, v.Y, 3);
        Assert.Equal(-1f, v.Z, 3);
    }

    // ========================================================================
    // 运算符测试
    // ========================================================================

    [Fact]
    public void Multiplication_CombinesRotations()
    {
        var a = Quaternion.FromAxisAngle(Vector3.Up, 45f);
        var b = Quaternion.FromAxisAngle(Vector3.Up, 45f);

        var combined = a * b;

        // 45 + 45 = 90度
        var v = Vector3.Right;
        var result = combined.RotateVector(v);

        Assert.Equal(0f, result.X, 3);
        Assert.Equal(0f, result.Y, 4);
        Assert.Equal(-1f, result.Z, 3);
    }

    [Fact]
    public void Multiplication_Vector_RotatesVector()
    {
        var q = Quaternion.FromAxisAngle(Vector3.Up, 90f);
        var v = Vector3.Right;

        var result = q * v;

        Assert.Equal(0f, result.X, 4);
        Assert.Equal(0f, result.Y, 4);
        Assert.Equal(-1f, result.Z, 4);
    }

    [Fact]
    public void Equality_WorksCorrectly()
    {
        var a = new Quaternion(1f, 2f, 3f, 4f);
        var b = new Quaternion(1f, 2f, 3f, 4f);
        var c = new Quaternion(5f, 6f, 7f, 8f);

        Assert.True(a == b);
        Assert.False(a == c);
        Assert.True(a != c);
    }
}
