using System.Reflection;
using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// 泛型组件类型缓存——从 <see cref="ComponentIdAttribute"/> 读取 TypeId，
/// 零开销静态缓存，每个 T 只解析一次。
/// 替代旧 <c>ComponentTypeRegistry&lt;T&gt;</c>（该类已废弃）。
/// </summary>
internal static class ComponentTypeCache<T> where T : struct
{
    /// <summary>组件的稳定 TypeId（来自 [ComponentId] 特性或 FNV-1a 回退）。</summary>
    public static readonly ulong TypeId = ResolveTypeId();

    /// <summary>组件大小（字节）。</summary>
    public static readonly int SizeBytes = Marshal.SizeOf<T>();

    private static ulong ResolveTypeId()
    {
        var attr = typeof(T).GetCustomAttribute<ComponentIdAttribute>();
        if (attr != null)
        {
            return attr.TypeId;
        }

        // 回退：FNV-1a of typeof(T).Name —— 仅用于无 [ComponentId] 的临时组件。
        // 生产代码应始终标注 [ComponentId]。
        return Fnv1a64(typeof(T).Name);
    }

    private static ulong Fnv1a64(string name)
    {
        ulong hash = 14695981039346656037UL;
        foreach (var c in name)
        {
            hash ^= (byte)c;
            hash *= 1099511628211UL;
        }
        return hash;
    }
}
