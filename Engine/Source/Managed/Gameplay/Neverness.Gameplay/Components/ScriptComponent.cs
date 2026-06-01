// ============================================================================
// ScriptComponent.cs - 脚本组件（ECS 侧）
// ============================================================================
// 存储脚本类型 ID 和 Behaviour 索引，不存储 Behaviour 实例。
// Behaviour 实例由 ScriptBehaviourScheduler 独立持有。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本组件（ECS 侧）：存储脚本类型 ID，不存储实例。
/// </summary>
/// <remarks>
/// ⚠️ 这是 Native ECS 的 struct 组件，与 C++ 层内存布局对齐（blittable）。
///
/// 字段说明：
/// - ScriptTypeId: 脚本类型 ID（FNV-1a hash），用于 ScriptRegistry 查找
/// - BehaviourIndex: Behaviour 实例在 Scheduler 中的索引（-1 表示未创建）
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
public struct ScriptComponent
{
    // ========================================================================
    // 数据字段
    // ========================================================================

    /// <summary>脚本类型 ID（FNV-1a hash）。</summary>
    public ulong ScriptTypeId;

    /// <summary>Behaviour 实例在 Scheduler 中的索引（-1 表示未创建）。</summary>
    public int BehaviourIndex;

    /// <summary>脚本是否启用。</summary>
    public byte Enabled;  // 使用 byte 代替 bool，与 C++ bool 对齐

    // ========================================================================
    // 常量
    // ========================================================================

    /// <summary>无效的 Behaviour 索引。</summary>
    public const int InvalidBehaviourIndex = -1;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建新的 ScriptComponent。</summary>
    /// <param name="scriptTypeId">脚本类型 ID。</param>
    public ScriptComponent(ulong scriptTypeId)
    {
        ScriptTypeId = scriptTypeId;
        BehaviourIndex = InvalidBehaviourIndex;
        Enabled = 1;  // 默认启用
    }

    // ========================================================================
    // 辅助方法
    // ========================================================================

    /// <summary>是否已创建 Behaviour 实例。</summary>
    public readonly bool HasBehaviour => BehaviourIndex != InvalidBehaviourIndex;

    /// <summary>是否启用。</summary>
    public readonly bool IsEnabled => Enabled != 0;

    /// <summary>设置启用状态。</summary>
    /// <param name="enabled">是否启用。</param>
    public void SetEnabled(bool enabled)
    {
        Enabled = enabled ? (byte)1 : (byte)0;
    }
}
