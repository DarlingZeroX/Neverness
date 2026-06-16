using System.Numerics;
using System.Runtime.InteropServices;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 变换组件——位置、旋转、缩放及世界矩阵。
/// 使用 System.Numerics 类型（SIMD 友好）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct TransformComponent : IComponent
{
    /// <summary>局部位置。</summary>
    public Vector3 Position;

    /// <summary>局部旋转（四元数，XYZW 顺序）。</summary>
    public Quaternion Rotation;

    /// <summary>局部缩放。</summary>
    public Vector3 Scale;

    /// <summary>世界矩阵（由 TransformSystem 每帧计算）。</summary>
    public Matrix4x4 WorldMatrix;

    /// <summary>创建默认变换（原点、无旋转、单位缩放）。</summary>
    public static TransformComponent Default => new()
    {
        Position = Vector3.Zero,
        Rotation = Quaternion.Identity,
        Scale = Vector3.One,
        WorldMatrix = Matrix4x4.Identity,
    };

    /// <summary>计算局部矩阵 TRS。</summary>
    public readonly Matrix4x4 ComputeLocalMatrix()
    {
        return Matrix4x4.CreateScale(Scale)
             * Matrix4x4.CreateFromQuaternion(Rotation)
             * Matrix4x4.CreateTranslation(Position);
    }
}
