// ============================================================================
// Vector3Tests.cs - Vector3 单元测试
// ============================================================================

using Neverness.Gameplay;
using Xunit;

namespace Neverness.Gameplay.Tests.Math;

/// <summary>
/// Vector3 单元测试。
/// </summary>
public class Vector3Tests
{
    // ========================================================================
    // 构造函数测试
    // ========================================================================

    [Fact]
    public void Constructor_SetsComponents()
    {
        var v = new Vector3(1f, 2f, 3f);

        Assert.Equal(1f, v.X);
        Assert.Equal(2f, v.Y);
        Assert.Equal(3f, v.Z);
    }

    [Fact]
    public void Constructor_SingleValue_SetsAllComponents()
    {
        var v = new Vector3(5f);

        Assert.Equal(5f, v.X);
        Assert.Equal(5f, v.Y);
        Assert.Equal(5f, v.Z);
    }

    // ========================================================================
    // 静态属性测试
    // ========================================================================

    [Fact]
    public void Zero_ReturnsZeroVector()
    {
        var v = Vector3.Zero;

        Assert.Equal(0f, v.X);
        Assert.Equal(0f, v.Y);
        Assert.Equal(0f, v.Z);
    }

    [Fact]
    public void One_ReturnsOneVector()
    {
        var v = Vector3.One;

        Assert.Equal(1f, v.X);
        Assert.Equal(1f, v.Y);
        Assert.Equal(1f, v.Z);
    }

    [Fact]
    public void Right_ReturnsCorrectVector()
    {
        var v = Vector3.Right;

        Assert.Equal(1f, v.X);
        Assert.Equal(0f, v.Y);
        Assert.Equal(0f, v.Z);
    }

    [Fact]
    public void Up_ReturnsCorrectVector()
    {
        var v = Vector3.Up;

        Assert.Equal(0f, v.X);
        Assert.Equal(1f, v.Y);
        Assert.Equal(0f, v.Z);
    }

    [Fact]
    public void Forward_ReturnsCorrectVector()
    {
        var v = Vector3.Forward;

        Assert.Equal(0f, v.X);
        Assert.Equal(0f, v.Y);
        Assert.Equal(-1f, v.Z);
    }

    // ========================================================================
    // 计算属性测试
    // ========================================================================

    [Fact]
    public void Magnitude_ReturnsCorrectLength()
    {
        var v = new Vector3(3f, 4f, 0f);

        Assert.Equal(5f, v.Magnitude, 4);
    }

    [Fact]
    public void SqrMagnitude_ReturnsCorrectSquaredLength()
    {
        var v = new Vector3(3f, 4f, 0f);

        Assert.Equal(25f, v.SqrMagnitude, 4);
    }

    [Fact]
    public void Normalized_ReturnsUnitVector()
    {
        var v = new Vector3(3f, 4f, 0f);
        var n = v.Normalized;

        Assert.Equal(1f, n.Magnitude, 4);
        Assert.Equal(0.6f, n.X, 4);
        Assert.Equal(0.8f, n.Y, 4);
    }

    [Fact]
    public void Normalized_ZeroVector_ReturnsZero()
    {
        var v = Vector3.Zero;
        var n = v.Normalized;

        Assert.Equal(0f, n.X);
        Assert.Equal(0f, n.Y);
        Assert.Equal(0f, n.Z);
    }

    // ========================================================================
    // 方法测试
    // ========================================================================

    [Fact]
    public void Dot_ReturnsCorrectValue()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(4f, 5f, 6f);

        var dot = Vector3.Dot(a, b);

        Assert.Equal(32f, dot, 4);
    }

    [Fact]
    public void Cross_ReturnsCorrectVector()
    {
        var a = Vector3.Right;
        var b = Vector3.Up;

        var cross = Vector3.Cross(a, b);

        Assert.Equal(0f, cross.X, 4);
        Assert.Equal(0f, cross.Y, 4);
        Assert.Equal(1f, cross.Z, 4);
    }

    [Fact]
    public void Distance_ReturnsCorrectDistance()
    {
        var a = new Vector3(0f, 0f, 0f);
        var b = new Vector3(3f, 4f, 0f);

        var distance = Vector3.Distance(a, b);

        Assert.Equal(5f, distance, 4);
    }

    [Fact]
    public void Lerp_InterpolatesCorrectly()
    {
        var a = Vector3.Zero;
        var b = Vector3.One;

        var result = Vector3.Lerp(a, b, 0.5f);

        Assert.Equal(0.5f, result.X, 4);
        Assert.Equal(0.5f, result.Y, 4);
        Assert.Equal(0.5f, result.Z, 4);
    }

    [Fact]
    public void Lerp_ClampsT()
    {
        var a = Vector3.Zero;
        var b = Vector3.One;

        var result = Vector3.Lerp(a, b, 2f);

        Assert.Equal(1f, result.X, 4);
        Assert.Equal(1f, result.Y, 4);
        Assert.Equal(1f, result.Z, 4);
    }

    // ========================================================================
    // 运算符测试
    // ========================================================================

    [Fact]
    public void Addition_WorksCorrectly()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(4f, 5f, 6f);

        var result = a + b;

        Assert.Equal(5f, result.X, 4);
        Assert.Equal(7f, result.Y, 4);
        Assert.Equal(9f, result.Z, 4);
    }

    [Fact]
    public void Subtraction_WorksCorrectly()
    {
        var a = new Vector3(4f, 5f, 6f);
        var b = new Vector3(1f, 2f, 3f);

        var result = a - b;

        Assert.Equal(3f, result.X, 4);
        Assert.Equal(3f, result.Y, 4);
        Assert.Equal(3f, result.Z, 4);
    }

    [Fact]
    public void ScalarMultiplication_WorksCorrectly()
    {
        var v = new Vector3(1f, 2f, 3f);

        var result = v * 2f;

        Assert.Equal(2f, result.X, 4);
        Assert.Equal(4f, result.Y, 4);
        Assert.Equal(6f, result.Z, 4);
    }

    [Fact]
    public void ScalarDivision_WorksCorrectly()
    {
        var v = new Vector3(2f, 4f, 6f);

        var result = v / 2f;

        Assert.Equal(1f, result.X, 4);
        Assert.Equal(2f, result.Y, 4);
        Assert.Equal(3f, result.Z, 4);
    }

    [Fact]
    public void Negation_WorksCorrectly()
    {
        var v = new Vector3(1f, -2f, 3f);

        var result = -v;

        Assert.Equal(-1f, result.X, 4);
        Assert.Equal(2f, result.Y, 4);
        Assert.Equal(-3f, result.Z, 4);
    }

    [Fact]
    public void Equality_WorksCorrectly()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(1f, 2f, 3f);
        var c = new Vector3(4f, 5f, 6f);

        Assert.True(a == b);
        Assert.False(a == c);
        Assert.True(a != c);
    }

    [Fact]
    public void Equals_WorksCorrectly()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(1f, 2f, 3f);
        var c = new Vector3(4f, 5f, 6f);

        Assert.True(a.Equals(b));
        Assert.False(a.Equals(c));
        Assert.False(a.Equals(null));
    }

    [Fact]
    public void GetHashCode_ReturnsSameForEqualVectors()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(1f, 2f, 3f);

        Assert.Equal(a.GetHashCode(), b.GetHashCode());
    }
}
