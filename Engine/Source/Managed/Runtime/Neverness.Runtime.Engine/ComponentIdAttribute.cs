using System;

namespace Neverness.Runtime.Engine;

/// <summary>
/// 组件稳定标识特性——显式声明组件的跨版本 TypeId。
/// 不依赖类型名，rename / namespace 变更不影响 ID。
/// </summary>
/// <remarks>
/// <see cref="TypeId"/> 须与 Native 端 <c>NN_REGISTER_COMPONENT</c> 的显式名称字符串的 FNV-1a 哈希一致。
/// <see cref="Name"/> 须与 Native 端 NameUtf8 参数完全一致。
/// </remarks>
[AttributeUsage(AttributeTargets.Struct, Inherited = false, AllowMultiple = false)]
public sealed class ComponentIdAttribute : Attribute
{
    /// <summary>稳定的 64-bit TypeId（FNV-1a 显式名称哈希）。</summary>
    public new ulong TypeId { get; }

    /// <summary>显式名称（须与 Native NN_REGISTER_COMPONENT 的 NameUtf8 一致）。</summary>
    public string? Name { get; set; }

    public ComponentIdAttribute(ulong typeId, string? name = null)
    {
        TypeId = typeId;
        Name = name;
    }
}
