using System.Numerics;
using System.Runtime.InteropServices;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 摄像机组件——投影参数和矩阵。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct CameraComponent : IComponent
{
    /// <summary>视野角度（弧度）。</summary>
    public float FieldOfView;

    /// <summary>宽高比。</summary>
    public float AspectRatio;

    /// <summary>近裁剪面。</summary>
    public float NearPlane;

    /// <summary>远裁剪面。</summary>
    public float FarPlane;

    /// <summary>是否正交投影。</summary>
    public bool IsOrthographic;

    /// <summary>正交大小（正交模式用）。</summary>
    public float OrthographicSize;

    /// <summary>投影矩阵（由 CameraSystem 计算）。</summary>
    public Matrix4x4 ProjectionMatrix;

    /// <summary>视图矩阵（由 CameraSystem 计算）。</summary>
    public Matrix4x4 ViewMatrix;

    /// <summary>创建默认透视摄像机。</summary>
    public static CameraComponent DefaultPerspective => new()
    {
        FieldOfView = MathF.PI / 4f, // 45 度
        AspectRatio = 16f / 9f,
        NearPlane = 0.1f,
        FarPlane = 1000f,
        IsOrthographic = false,
        OrthographicSize = 10f,
        ProjectionMatrix = Matrix4x4.Identity,
        ViewMatrix = Matrix4x4.Identity,
    };

    /// <summary>创建默认正交摄像机。</summary>
    public static CameraComponent DefaultOrthographic => new()
    {
        FieldOfView = MathF.PI / 4f,
        AspectRatio = 16f / 9f,
        NearPlane = 0.1f,
        FarPlane = 1000f,
        IsOrthographic = true,
        OrthographicSize = 10f,
        ProjectionMatrix = Matrix4x4.Identity,
        ViewMatrix = Matrix4x4.Identity,
    };
}
