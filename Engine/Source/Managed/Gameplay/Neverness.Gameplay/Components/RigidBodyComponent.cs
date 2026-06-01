// ============================================================================
// RigidBodyComponent.cs - 刚体组件
// ============================================================================
// 刚体组件，定义物理模拟参数。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 刚体组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct RigidBodyComponent
{
    /// <summary>质量。</summary>
    public float Mass;

    /// <summary>阻力。</summary>
    public float Drag;

    /// <summary>角阻力。</summary>
    public float AngularDrag;

    /// <summary>是否使用重力。</summary>
    public byte UseGravity;  // byte 对齐 C++ bool

    /// <summary>是否运动学（不受物理力影响）。</summary>
    public byte IsKinematic;

    /// <summary>线性速度。</summary>
    public Vector3 Velocity;

    /// <summary>角速度。</summary>
    public Vector3 AngularVelocity;

    /// <summary>惯性张量。</summary>
    public Vector3 InertiaTensor;

    /// <summary>质心偏移。</summary>
    public Vector3 CenterOfMass;

    /// <summary>约束标志。</summary>
    public RigidbodyConstraints Constraints;

    /// <summary>碰撞检测模式。</summary>
    public CollisionDetectionMode CollisionDetection;

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认刚体组件。</summary>
    public static readonly RigidBodyComponent Default = new()
    {
        Mass = 1f,
        Drag = 0f,
        AngularDrag = 0.05f,
        UseGravity = 1,
        IsKinematic = 0,
        Velocity = Vector3.Zero,
        AngularVelocity = Vector3.Zero,
        InertiaTensor = Vector3.One,
        CenterOfMass = Vector3.Zero,
        Constraints = RigidbodyConstraints.None,
        CollisionDetection = CollisionDetectionMode.Discrete
    };
}

/// <summary>
/// 刚体约束标志。
/// </summary>
[Flags]
public enum RigidbodyConstraints
{
    /// <summary>无约束。</summary>
    None = 0,

    /// <summary>冻结位置 X。</summary>
    FreezePositionX = 1,

    /// <summary>冻结位置 Y。</summary>
    FreezePositionY = 2,

    /// <summary>冻结位置 Z。</summary>
    FreezePositionZ = 4,

    /// <summary>冻结旋转 X。</summary>
    FreezeRotationX = 8,

    /// <summary>冻结旋转 Y。</summary>
    FreezeRotationY = 16,

    /// <summary>冻结旋转 Z。</summary>
    FreezeRotationZ = 32,

    /// <summary>冻结所有位置。</summary>
    FreezeAllPosition = FreezePositionX | FreezePositionY | FreezePositionZ,

    /// <summary>冻结所有旋转。</summary>
    FreezeAllRotation = FreezeRotationX | FreezeRotationY | FreezeRotationZ,

    /// <summary>冻结所有。</summary>
    FreezeAll = FreezeAllPosition | FreezeAllRotation
}

/// <summary>
/// 碰撞检测模式。
/// </summary>
public enum CollisionDetectionMode
{
    /// <summary>离散碰撞检测。</summary>
    Discrete = 0,

    /// <summary>连续碰撞检测。</summary>
    Continuous = 1,

    /// <summary>连续动态碰撞检测。</summary>
    ContinuousDynamic = 2,

    /// <summary>连续推测碰撞检测。</summary>
    ContinuousSpeculative = 3
}
