using System.Reflection;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Managed System 调度器——注册、拓扑排序、按 TickGroup 分组执行。
/// 由 <see cref="SceneWorld.Systems"/> 持有。
/// </summary>
public sealed class SceneSystemScheduler : IDisposable
{
    /// <summary>所属世界。</summary>
    private readonly SceneWorld _world;

    /// <summary>所有已注册 System（保持注册顺序）。</summary>
    private readonly List<ISceneSystem> _allSystems = [];

    /// <summary>按 TickGroup 分组的已排序 System 列表（延迟构建）。</summary>
    private SortedDictionary<int, List<ISceneSystem>>? _groupedSystems;

    /// <summary>是否需要重新排序。</summary>
    private bool _needsRebuild = true;

    /// <summary>已初始化的 System 集合。</summary>
    private readonly HashSet<ISceneSystem> _initialized = [];

    public SceneSystemScheduler(SceneWorld world)
    {
        _world = world;
    }

    /// <summary>已注册 System 数量。</summary>
    public int Count => _allSystems.Count;

    /// <summary>注册 System。注册后首次 Tick 前自动 Initialize。</summary>
    public void Register(ISceneSystem system)
    {
        ArgumentNullException.ThrowIfNull(system);

        if (_allSystems.Contains(system))
        {
            return;
        }

        _allSystems.Add(system);
        _needsRebuild = true;
    }

    /// <summary>注册多个 System。</summary>
    public void RegisterAll(params ISceneSystem[] systems)
    {
        foreach (var system in systems)
        {
            Register(system);
        }
    }

    /// <summary>移除已注册 System。</summary>
    public void Unregister(ISceneSystem system)
    {
        ArgumentNullException.ThrowIfNull(system);

        if (_allSystems.Remove(system))
        {
            _needsRebuild = true;
        }
    }

    /// <summary>按 TickGroup 驱动 Tick 系统。</summary>
    public void Tick(TickGroup group, float deltaTime)
    {
        EnsureBuilt();
        if (!_groupedSystems!.TryGetValue((int)group, out var systems))
        {
            return;
        }

        foreach (var system in systems)
        {
            EnsureInitialized(system);
            if (system is ISystemTick tick)
            {
                tick.Tick(_world, deltaTime);
            }
        }
    }

    /// <summary>按 TickGroup 驱动 FixedTick 系统。</summary>
    public void FixedTick(TickGroup group, float fixedDeltaTime)
    {
        EnsureBuilt();
        if (!_groupedSystems!.TryGetValue((int)group, out var systems))
        {
            return;
        }

        foreach (var system in systems)
        {
            EnsureInitialized(system);
            if (system is ISystemFixedTick tick)
            {
                tick.FixedTick(_world, fixedDeltaTime);
            }
        }
    }

    /// <summary>按 TickGroup 驱动 LateTick 系统。</summary>
    public void LateTick(TickGroup group, float deltaTime)
    {
        EnsureBuilt();
        if (!_groupedSystems!.TryGetValue((int)group, out var systems))
        {
            return;
        }

        foreach (var system in systems)
        {
            EnsureInitialized(system);
            if (system is ISystemLateTick tick)
            {
                tick.LateTick(_world, deltaTime);
            }
        }
    }

    /// <summary>
    /// 重建调度器——关闭所有现有 System，允许重新注册。
    /// 用于热重载场景：旧程序集卸载后，新程序集重新注册 System。
    /// </summary>
    public void Rebuild()
    {
        Dispose();
    }

    /// <summary>关闭所有 System 并清空注册。</summary>
    public void Dispose()
    {
        // 逆序 Shutdown
        for (var i = _allSystems.Count - 1; i >= 0; i--)
        {
            if (_allSystems[i] is ISystemShutdown shutdown)
            {
                shutdown.Shutdown(_world);
            }
        }

        _allSystems.Clear();
        _initialized.Clear();
        _groupedSystems?.Clear();
        _needsRebuild = true;
    }

    // ── 内部 ──

    private void EnsureInitialized(ISceneSystem system)
    {
        if (_initialized.Add(system) && system is ISystemInitialize init)
        {
            init.Initialize(_world);
        }
    }

    private void EnsureBuilt()
    {
        if (!_needsRebuild)
        {
            return;
        }

        _needsRebuild = false;
        _groupedSystems = BuildGroupedSystems();
    }

    private SortedDictionary<int, List<ISceneSystem>> BuildGroupedSystems()
    {
        // 1. 拓扑排序
        var sorted = TopologicalSort();

        // 2. 按 TickGroup 分组
        var grouped = new SortedDictionary<int, List<ISceneSystem>>();

        foreach (var system in sorted)
        {
            var group = GetTickGroup(system);
            var key = (int)group;

            if (!grouped.TryGetValue(key, out var list))
            {
                list = new List<ISceneSystem>();
                grouped[key] = list;
            }

            list.Add(system);
        }

        return grouped;
    }

    /// <summary>获取 System 的 TickGroup。</summary>
    private static TickGroup GetTickGroup(ISceneSystem system)
    {
        if (system is ISystemTick tick) return tick.TickGroup;
        if (system is ISystemFixedTick ft) return ft.TickGroup;
        if (system is ISystemLateTick lt) return lt.TickGroup;
        return TickGroup.Update;
    }

    /// <summary>
    /// 基于 <see cref="SystemDependencyAttribute"/> 的 Kahn 拓扑排序。
    /// 无依赖的 System 保持注册顺序；有依赖的保证被依赖者排在前面。
    /// </summary>
    private List<ISceneSystem> TopologicalSort()
    {
        // 构建依赖图
        var typeToSystem = new Dictionary<Type, ISceneSystem>();
        foreach (var s in _allSystems)
        {
            typeToSystem[s.GetType()] = s;
        }

        // 入度表
        var inDegree = new Dictionary<ISceneSystem, int>();
        // 邻接表：dependency → dependents
        var dependents = new Dictionary<ISceneSystem, List<ISceneSystem>>();

        foreach (var s in _allSystems)
        {
            inDegree.TryAdd(s, 0);
            dependents.TryAdd(s, []);

            var attrs = s.GetType().GetCustomAttributes<SystemDependencyAttribute>();
            foreach (var attr in attrs)
            {
                if (!typeToSystem.TryGetValue(attr.DependsOn, out var dep))
                {
                    continue; // 依赖未注册，忽略
                }

                inDegree[s] = inDegree.GetValueOrDefault(s) + 1;
                dependents[dep].Add(s);
            }
        }

        // Kahn 算法：入度为 0 的先入队（保持注册顺序）
        var queue = new Queue<ISceneSystem>();
        foreach (var s in _allSystems)
        {
            if (inDegree[s] == 0)
            {
                queue.Enqueue(s);
            }
        }

        var result = new List<ISceneSystem>(_allSystems.Count);
        while (queue.Count > 0)
        {
            var current = queue.Dequeue();
            result.Add(current);

            foreach (var dep in dependents[current])
            {
                inDegree[dep]--;
                if (inDegree[dep] == 0)
                {
                    queue.Enqueue(dep);
                }
            }
        }

        // 检测循环依赖（理论上不应发生）
        if (result.Count != _allSystems.Count)
        {
            throw new InvalidOperationException(
                $"Circular dependency detected among {_allSystems.Count} systems. " +
                $"Only {result.Count} could be sorted.");
        }

        return result;
    }
}
