// ============================================================================
// Mathf.cs - 数学工具
// ============================================================================
// 数学工具类，提供常用的数学函数。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 数学工具类。
/// </summary>
public static class Mathf
{
    // ========================================================================
    // 常量
    // ========================================================================

    /// <summary>圆周率 π。</summary>
    public const float PI = 3.14159265358979323846f;

    /// <summary>2π。</summary>
    public const float TwoPI = PI * 2f;

    /// <summary>π/2。</summary>
    public const float HalfPI = PI * 0.5f;

    /// <summary>度到弧度的转换因子。</summary>
    public const float Deg2Rad = PI / 180f;

    /// <summary>弧度到度的转换因子。</summary>
    public const float Rad2Deg = 180f / PI;

    /// <summary>浮点数 epsilon。</summary>
    public const float Epsilon = 1.175494351e-38f;

    // ========================================================================
    // 方法
    // ========================================================================

    /// <summary>绝对值。</summary>
    public static float Abs(float value) => MathF.Abs(value);

    /// <summary>绝对值。</summary>
    public static int Abs(int value) => Math.Abs(value);

    /// <summary>最小值。</summary>
    public static float Min(float a, float b) => MathF.Min(a, b);

    /// <summary>最小值。</summary>
    public static int Min(int a, int b) => Math.Min(a, b);

    /// <summary>最大值。</summary>
    public static float Max(float a, float b) => MathF.Max(a, b);

    /// <summary>最大值。</summary>
    public static int Max(int a, int b) => Math.Max(a, b);

    /// <summary>限制值在指定范围内。</summary>
    public static float Clamp(float value, float min, float max) => Math.Clamp(value, min, max);

    /// <summary>限制值在指定范围内。</summary>
    public static int Clamp(int value, int min, int max) => Math.Clamp(value, min, max);

    /// <summary>限制值在 0-1 范围内。</summary>
    public static float Clamp01(float value) => Clamp(value, 0f, 1f);

    /// <summary>线性插值。</summary>
    public static float Lerp(float a, float b, float t) => a + (b - a) * Clamp01(t);

    /// <summary>线性插值（不限制 t）。</summary>
    public static float LerpUnclamped(float a, float b, float t) => a + (b - a) * t;

    /// <summary>反向线性插值。</summary>
    public static float InverseLerp(float a, float b, float value)
    {
        if (MathF.Abs(a - b) < Epsilon)
            return 0f;
        return Clamp01((value - a) / (b - a));
    }

    /// <summary>平滑步进插值。</summary>
    public static float SmoothStep(float edge0, float edge1, float x)
    {
        var t = Clamp01((x - edge0) / (edge1 - edge0));
        return t * t * (3f - 2f * t);
    }

    /// <summary>正弦。</summary>
    public static float Sin(float radians) => MathF.Sin(radians);

    /// <summary>余弦。</summary>
    public static float Cos(float radians) => MathF.Cos(radians);

    /// <summary>正切。</summary>
    public static float Tan(float radians) => MathF.Tan(radians);

    /// <summary>反正弦。</summary>
    public static float Asin(float value) => MathF.Asin(value);

    /// <summary>反余弦。</summary>
    public static float Acos(float value) => MathF.Acos(value);

    /// <summary>反正切。</summary>
    public static float Atan(float value) => MathF.Atan(value);

    /// <summary>反正切（y/x）。</summary>
    public static float Atan2(float y, float x) => MathF.Atan2(y, x);

    /// <summary>平方根。</summary>
    public static float Sqrt(float value) => MathF.Sqrt(value);

    /// <summary>幂。</summary>
    public static float Pow(float x, float y) => MathF.Pow(x, y);

    /// <summary>自然对数。</summary>
    public static float Log(float value) => MathF.Log(value);

    /// <summary>以 10 为底的对数。</summary>
    public static float Log10(float value) => MathF.Log10(value);

    /// <summary>向上取整。</summary>
    public static float Ceil(float value) => MathF.Ceiling(value);

    /// <summary>向下取整。</summary>
    public static float Floor(float value) => MathF.Floor(value);

    /// <summary>四舍五入。</summary>
    public static float Round(float value) => MathF.Round(value);

    /// <summary>符号函数。</summary>
    public static float Sign(float value) => MathF.Sign(value);

    /// <summary>判断两个浮点数是否近似相等。</summary>
    public static bool Approximately(float a, float b, float tolerance = Epsilon)
    {
        return MathF.Abs(a - b) < tolerance;
    }

    /// <summary>将角度归一化到 -180 到 180 度范围。</summary>
    public static float NormalizeAngle(float angle)
    {
        while (angle > 180f) angle -= 360f;
        while (angle < -180f) angle += 360f;
        return angle;
    }

    /// <summary>将弧度归一化到 -π 到 π 范围。</summary>
    public static float NormalizeRadians(float radians)
    {
        while (radians > PI) radians -= TwoPI;
        while (radians < -PI) radians += TwoPI;
        return radians;
    }

    /// <summary>移动到目标值（限制最大变化量）。</summary>
    public static float MoveTowards(float current, float target, float maxDelta)
    {
        if (MathF.Abs(target - current) <= maxDelta)
            return target;
        return current + MathF.Sign(target - current) * maxDelta;
    }

    /// <summary>平滑阻尼。</summary>
    public static float SmoothDamp(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
    {
        smoothTime = MathF.Max(0.0001f, smoothTime);
        var omega = 2f / smoothTime;
        var x = omega * deltaTime;
        var exp = 1f / (1f + x + 0.48f * x * x + 0.235f * x * x * x);

        var change = current - target;
        var originalTo = target;

        var maxChange = maxSpeed * smoothTime;
        change = Clamp(change, -maxChange, maxChange);
        target = current - change;

        var temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        var output = target + (change + temp) * exp;

        if (originalTo - current > 0f == output > originalTo)
        {
            output = originalTo;
            currentVelocity = (output - originalTo) / deltaTime;
        }

        return output;
    }
}
