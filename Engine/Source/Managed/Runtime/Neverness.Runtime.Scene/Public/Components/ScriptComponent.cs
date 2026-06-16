using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 脚本组件——关联脚本类型。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ScriptComponent : IComponent
{
    /// <summary>脚本类型 ID（FNV-1a hash）。</summary>
    public ulong ScriptTypeId;

    /// <summary>脚本实例 ID（运行时用）。</summary>
    [Transient]
    public int InstanceId;

    /// <summary>是否已初始化。</summary>
    [Transient]
    public bool IsInitialized;

    /// <summary>创建脚本组件。</summary>
    public static ScriptComponent Create(ulong scriptTypeId)
    {
        return new ScriptComponent
        {
            ScriptTypeId = scriptTypeId,
            InstanceId = 0,
            IsInitialized = false,
        };
    }
}
