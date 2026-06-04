// ============================================================================
// TransformComponent.cs - 变换组件
// ============================================================================
// 位置、旋转、缩放组件，与 Native TransformComponent 内存布局对齐。
// ============================================================================

using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Gameplay;

/// <summary>
/// 变换组件：位置、旋转、缩放。
/// </summary>
/// <remarks>
/// ⚠️ 与 Native TransformComponent 内存布局对齐（blittable）。
/// ComponentTypeId = FNV1a64("Transform")，须与 Native BuiltinComponentRegistration.cpp 一致。
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
[ComponentId(0xC1FFF4F356DFB2FB, Name = "Transform")]
public struct TransformComponent
{
    // ========================================================================
    // 数据字段
    // ========================================================================

    /// <summary>本地位置。</summary>
    public Vector3 Position;

    /// <summary>本地旋转（四元数）。</summary>
    public Quaternion Rotation;

    /// <summary>本地缩放。</summary>
    public Vector3 Scale;

    /// <summary>世界变换矩阵（由 TransformSystem 每帧计算，只读）。</summary>
    public Matrix4x4 WorldMatrix;

    // ========================================================================
    // 计算属性
    // ========================================================================

    /// <summary>前方向量（本地坐标系）。</summary>
    public readonly Vector3 Forward => Rotation * Vector3.Forward;

    /// <summary>右方向量（本地坐标系）。</summary>
    public readonly Vector3 Right => Rotation * Vector3.Right;

    /// <summary>上方向量（本地坐标系）。</summary>
    public readonly Vector3 Up => Rotation * Vector3.Up;

    // ========================================================================
    // 变换方法
    // ========================================================================

    /// <summary>
    /// 平移（本地空间）。
    /// </summary>
    /// <param name="translation">平移向量。</param>
    public void Translate(Vector3 translation)
    {
        Position += translation;
    }

    /// <summary>
    /// 平移（世界空间）。
    /// </summary>
    /// <param name="translation">平移向量。</param>
    /// <param name="relativeTo">参考空间。</param>
    public void Translate(Vector3 translation, Space relativeTo)
    {
        if (relativeTo == Space.World)
        {
            Position += translation;
        }
        else
        {
            Position += Rotation * translation;
        }
    }

    /// <summary>
    /// 旋转（欧拉角，度）。
    /// </summary>
    /// <param name="eulerAngles">欧拉角（度）。</param>
    public void Rotate(Vector3 eulerAngles)
    {
        var rotation = Quaternion.FromEuler(eulerAngles);
        Rotation = Rotation * rotation;
    }

    /// <summary>
    /// 朝向目标点。
    /// </summary>
    /// <param name="target">目标位置。</param>
    public void LookAt(Vector3 target)
    {
        var direction = target - Position;
        if (direction.SqrMagnitude < 0.0001f)
            return;

        Rotation = Quaternion.LookRotation(direction, Vector3.Up);
    }

    /// <summary>
    /// 朝向目标点，指定上方向。
    /// </summary>
    /// <param name="target">目标位置。</param>
    /// <param name="up">上方向。</param>
    public void LookAt(Vector3 target, Vector3 up)
    {
        var direction = target - Position;
        if (direction.SqrMagnitude < 0.0001f)
            return;

        Rotation = Quaternion.LookRotation(direction, up);
    }

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认变换（位置原点，无旋转，缩放为1）。</summary>
    public static readonly TransformComponent Default = new()
    {
        Position = Vector3.Zero,
        Rotation = Quaternion.Identity,
        Scale = Vector3.One,
        WorldMatrix = Matrix4x4.Identity
    };
}

/// <summary>
/// 空间类型。
/// </summary>
public enum Space
{
    /// <summary>世界空间。</summary>
    World,

    /// <summary>本地空间。</summary>
    Self
}
