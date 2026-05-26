using System.Reflection;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 窗口类型注册表。
///
/// 支持手动注册和程序集反射发现。
/// 插件可通过 DiscoverFromAssemblies 动态注册窗口类型。
/// </summary>
public sealed class ImWindowRegistry
{
    private readonly Dictionary<string, Type> m_RegisteredTypes = new(StringComparer.Ordinal);

    /// <summary>注册窗口类型，使用 FullName 作为 key。</summary>
    public void Register<T>() where T : ImWindow, new()
    {
        m_RegisteredTypes[typeof(T).FullName!] = typeof(T);
    }

    /// <summary>注册窗口类型，使用自定义 key。</summary>
    public void Register<T>(string key) where T : ImWindow, new()
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(key);
        m_RegisteredTypes[key] = typeof(T);
    }

    /// <summary>注册窗口类型（Type 版本）。</summary>
    public void Register(Type windowType, string? key = null)
    {
        ArgumentNullException.ThrowIfNull(windowType);

        if (!typeof(ImWindow).IsAssignableFrom(windowType))
            throw new ArgumentException($"类型 {windowType.Name} 不是 ImWindow 的子类。");

        m_RegisteredTypes[key ?? windowType.FullName!] = windowType;
    }

    /// <summary>
    /// 从程序集中自动发现所有非抽象 ImWindow 子类。
    /// 插件用法：registry.DiscoverFromAssemblies(pluginAssembly)
    /// </summary>
    public void DiscoverFromAssemblies(params Assembly[] assemblies)
    {
        foreach (var asm in assemblies)
        {
            foreach (var type in asm.GetTypes())
            {
                if (type.IsAbstract || type.IsInterface) continue;
                if (!typeof(ImWindow).IsAssignableFrom(type)) continue;
                if (type == typeof(ImWindow)) continue;

                m_RegisteredTypes.TryAdd(type.FullName!, type);
            }
        }
    }

    /// <summary>通过 key 创建窗口实例。</summary>
    public ImWindow? CreateInstance(string key)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(key);

        return m_RegisteredTypes.TryGetValue(key, out var type)
            ? (ImWindow)Activator.CreateInstance(type)!
            : null;
    }

    /// <summary>尝试获取已注册的类型。</summary>
    public bool TryGetType(string key, out Type type)
    {
        return m_RegisteredTypes.TryGetValue(key, out type!);
    }

    /// <summary>所有已注册的窗口类型。</summary>
    public IReadOnlyDictionary<string, Type> RegisteredTypes => m_RegisteredTypes;

    /// <summary>已注册的类型数量。</summary>
    public int Count => m_RegisteredTypes.Count;
}
