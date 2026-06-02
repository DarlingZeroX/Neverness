// ============================================================================
// ScriptComponent.cs - 脚本组件（Gameplay 层）
// ============================================================================
// 关联 Entity 与 C# 脚本类型，只保存可序列化状态。
// 运行时实例映射由 ScriptBehaviourScheduler + BehaviourRegistry 管理。
//
// ⚠️ ComponentTypeId 与 ScriptTypeId 是完全不同的概念：
// - ComponentTypeId = FNV1a64("Script")（组件类型标识，用于 ECS 注册/ABI 调用）
// - ScriptTypeId = FNV1a64(Type.FullName)（脚本类标识，用于实例化/序列化持久化）
//
// 内存布局与 Native NNScriptComponent 对齐（16 字节）：
//   ScriptTypeId  uint64  8B
//   Enabled       byte    1B（Gameplay 层用 bool，ABI 转换在边界层）
//   _reserved     byte[7] 7B
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本组件（Gameplay 层）：关联 Entity 与 C# 脚本类型。
/// </summary>
/// <remarks>
/// ⚠️ 这是 Gameplay 包装层，不直接参与 ABI。
/// ABI 层使用 NNScriptComponentData（Neverness.Runtime.Engine）。
///
/// 当前版本限制：一个 Entity 最多挂载一个 ScriptComponent。
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
public struct ScriptComponent
{
    // ========================================================================
    // 数据字段
    // ========================================================================

    /// <summary>脚本类型 ID（FNV1a64(Type.FullName)）。</summary>
    public ulong ScriptTypeId;

    /// <summary>脚本是否启用。</summary>
    /// <remarks>
    /// Gameplay 层使用 bool 提供友好的用户 API。
    /// 写入 Native ECS 时转换为 byte（Enabled ? (byte)1 : (byte)0）。
    /// </remarks>
    public bool Enabled;

    // ========================================================================
    // 对齐填充（与 Native 16 字节布局对齐）
    // ========================================================================

    /// <summary>对齐填充，使总大小为 16 字节。</summary>
    private readonly byte _reserved0;
    private readonly byte _reserved1;
    private readonly byte _reserved2;
    private readonly byte _reserved3;
    private readonly byte _reserved4;
    private readonly byte _reserved5;
    private readonly byte _reserved6;

    // ========================================================================
    // 构造函数
    // ========================================================================

    /// <summary>创建新的 ScriptComponent。</summary>
    /// <param name="scriptTypeId">脚本类型 ID（FNV1a64(Type.FullName)）。</param>
    public ScriptComponent(ulong scriptTypeId)
    {
        ScriptTypeId = scriptTypeId;
        Enabled = true;  // 默认启用
    }

    // ========================================================================
    // 辅助方法
    // ========================================================================

    /// <summary>是否已设置脚本类型。</summary>
    public readonly bool HasScript => ScriptTypeId != 0;

    /// <summary>设置启用状态。</summary>
    /// <param name="enabled">是否启用。</param>
    public void SetEnabled(bool enabled)
    {
        Enabled = enabled;
    }
}
