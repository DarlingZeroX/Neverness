using System.Reflection;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 资产工厂注册表——扫描所有已加载程序集中的 <see cref="IAssetFactory"/> 实现并注册。
/// </summary>
public sealed class AssetFactoryRegistry
{
    ////////////////////////////////////////////////////////////////
    /// Singleton
    ////////////////////////////////////////////////////////////////

    public static AssetFactoryRegistry Instance { get; } = new();

    ////////////////////////////////////////////////////////////////
    /// Private Fields
    ////////////////////////////////////////////////////////////////

    private readonly List<IAssetFactory> _factories = [];
    private bool _discovered;

    ////////////////////////////////////////////////////////////////
    /// Public API
    ////////////////////////////////////////////////////////////////

    /// <summary>获取所有已注册的工厂（首次访问时自动执行发现）。</summary>
    public IReadOnlyList<IAssetFactory> Factories
    {
        get
        {
            EnsureDiscovered();
            return _factories;
        }
    }

    /// <summary>手动注册一个工厂实例。</summary>
    public void Register(IAssetFactory factory)
    {
        ArgumentNullException.ThrowIfNull(factory);
        if (!_factories.Any(f => f.GetType() == factory.GetType()))
        {
            _factories.Add(factory);
        }
    }

    /// <summary>按分类获取工厂列表。</summary>
    public IReadOnlyList<IAssetFactory> GetByCategory(string category)
    {
        EnsureDiscovered();
        return _factories
            .Where(f => string.Equals(f.Category, category, StringComparison.OrdinalIgnoreCase))
            .ToList();
    }

    /// <summary>按显示名称查找工厂。</summary>
    public IAssetFactory? FindByName(string displayName)
    {
        EnsureDiscovered();
        return _factories.FirstOrDefault(
            f => string.Equals(f.DisplayName, displayName, StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>强制触发重新发现（用于 Hot Reload 场景）。</summary>
    public void Rediscover()
    {
        _factories.Clear();
        _discovered = false;
        EnsureDiscovered();
    }

    ////////////////////////////////////////////////////////////////
    /// Auto-Discovery
    ////////////////////////////////////////////////////////////////

    private void EnsureDiscovered()
    {
        if (_discovered)
            return;

        _discovered = true;
        DiscoverFromAssemblies();
    }

    private void DiscoverFromAssemblies()
    {
        // 扫描所有已加载的 Editor 程序集
        foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            // 跳过系统程序集和动态程序集
            if (assembly.IsDynamic)
                continue;

            // 仅扫描 Neverness.Editor.* 程序集（避免扫描 Runtime/System 等）
            var name = assembly.GetName().Name;
            if (name == null || !name.StartsWith("Neverness.Editor.", StringComparison.Ordinal))
                continue;

            ScanAssembly(assembly);
        }
    }

    private void ScanAssembly(Assembly assembly)
    {
        Type[] types;
        try
        {
            types = assembly.GetTypes();
        }
        catch (ReflectionTypeLoadException ex)
        {
            // 部分类型加载失败时，使用成功加载的类型
            types = ex.Types.Where(t => t != null).ToArray()!;
        }

        foreach (var type in types)
        {
            if (type.IsAbstract || type.IsInterface)
                continue;

            if (!typeof(IAssetFactory).IsAssignableFrom(type))
                continue;

            try
            {
                if (Activator.CreateInstance(type) is IAssetFactory factory)
                {
                    Register(factory);
                }
            }
            catch
            {
                // 工厂创建失败时跳过（构造函数可能有未满足的依赖）
            }
        }
    }
}
