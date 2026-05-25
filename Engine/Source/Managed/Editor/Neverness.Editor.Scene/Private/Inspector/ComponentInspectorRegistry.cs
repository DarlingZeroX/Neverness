using System.Reflection;

namespace Neverness.Editor.Scene.Private.Inspector;

/// <summary>
/// 组件检查器注册表——按 TypeId 索引，支持自动程序集扫描与手动注册。
/// <see cref="DetailInspector"/> 通过此注册表枚举和分发组件绘制。
/// </summary>
public static class ComponentInspectorRegistry
{
    /// <summary>TypeId → Inspector 映射。</summary>
    private static readonly Dictionary<ulong, IComponentInspector> s_inspectors = new();

    /// <summary>所有已注册检查器的有序列表（按 Order → DisplayName 排序，UI 枚举用）。</summary>
    private static readonly List<IComponentInspector> s_ordered = new();

    /// <summary>是否已完成自动扫描。</summary>
    private static bool s_discovered;

    /// <summary>所有已注册检查器的只读视图（按 DisplayName 排序）。</summary>
    public static IReadOnlyList<IComponentInspector> Inspectors
    {
        get
        {
            EnsureDiscovered();
            return s_ordered;
        }
    }

    /// <summary>手动注册一个组件检查器。</summary>
    public static void Register(IComponentInspector inspector)
    {
        ArgumentNullException.ThrowIfNull(inspector);
        if (inspector.ComponentTypeId == 0)
            throw new ArgumentException("Inspector 的 ComponentTypeId 不能为零。");

        s_inspectors[inspector.ComponentTypeId] = inspector;

        // 维护有序列表（按 Order 升序，同 Order 按 DisplayName 字典序）
        s_ordered.RemoveAll(i => i.ComponentTypeId == inspector.ComponentTypeId);
        s_ordered.Add(inspector);
        s_ordered.Sort((a, b) =>
        {
            int cmp = a.Order.CompareTo(b.Order);
            return cmp != 0 ? cmp : string.Compare(a.DisplayName, b.DisplayName, StringComparison.Ordinal);
        });
    }

    /// <summary>按 TypeId 查找检查器。</summary>
    public static IComponentInspector? GetInspector(ulong typeId)
    {
        EnsureDiscovered();
        return s_inspectors.GetValueOrDefault(typeId);
    }

    /// <summary>按类型查找检查器。</summary>
    public static IComponentInspector? GetInspector<T>() where T : struct
    {
        EnsureDiscovered();
        // 通过枚举查找匹配的 CLR 类型
        foreach (var inspector in s_inspectors.Values)
        {
            if (inspector.ClrType == typeof(T))
                return inspector;
        }
        return null;
    }

    /// <summary>
    /// 扫描指定程序集中所有 <see cref="IComponentInspector"/> 的非抽象实现并注册。
    /// </summary>
    public static void DiscoverFromAssembly(Assembly assembly)
    {
        ArgumentNullException.ThrowIfNull(assembly);

        foreach (var type in assembly.GetTypes())
        {
            if (type.IsAbstract || type.IsInterface)
                continue;
            if (!typeof(IComponentInspector).IsAssignableFrom(type))
                continue;

            try
            {
                if (Activator.CreateInstance(type) is IComponentInspector inspector)
                    Register(inspector);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(
                    $"[ComponentInspectorRegistry] 无法创建 {type.FullName}: {ex.Message}");
            }
        }
    }

    /// <summary>确保已执行自动发现（惰性，仅首次调用时扫描）。</summary>
    private static void EnsureDiscovered()
    {
        if (s_discovered) return;
        s_discovered = true;

        // 扫描当前程序集（Neverness.Editor.Scene）
        DiscoverFromAssembly(typeof(ComponentInspectorRegistry).Assembly);
    }
}
