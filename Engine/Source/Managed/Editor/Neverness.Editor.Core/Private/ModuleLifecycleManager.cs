using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// 模块生命周期管理器——按依赖拓扑排序初始化 Feature。
/// 支持自动发现（Assembly scanning）和手动注册。
/// </summary>
public sealed class ModuleLifecycleManager
{
    private readonly List<IEditorFeature> _features = new();
    private readonly Dictionary<string, IEditorFeature> _featureMap = new(StringComparer.Ordinal);
    private readonly HashSet<string> _initialized = new(StringComparer.Ordinal);

    /// <summary>已注册的 Feature 数量。</summary>
    public int Count => _features.Count;

    /// <summary>注册 Feature。</summary>
    public void RegisterFeature(IEditorFeature feature)
    {
        ArgumentNullException.ThrowIfNull(feature);

        if (_featureMap.ContainsKey(feature.FeatureId))
        {
            throw new InvalidOperationException(
                $"Feature '{feature.FeatureId}' is already registered.");
        }

        _features.Add(feature);
        _featureMap[feature.FeatureId] = feature;
    }

    /// <summary>
    /// 从程序集扫描 IEditorFeature 实现并自动注册。
    /// 未来插件只需将 DLL 放入 Plugins/ 目录即可被发现。
    /// </summary>
    public void DiscoverFeatures(params System.Reflection.Assembly[] assemblies)
    {
        foreach (var asm in assemblies)
        {
            foreach (var type in asm.GetTypes())
            {
                if (type.IsAbstract || type.IsInterface)
                    continue;

                if (!typeof(IEditorFeature).IsAssignableFrom(type))
                    continue;

                if (Activator.CreateInstance(type) is IEditorFeature feature)
                {
                    RegisterFeature(feature);
                }
            }
        }
    }

    /// <summary>按依赖拓扑排序初始化所有 Feature。</summary>
    public void InitializeAll(IEditorContext context)
    {
        var sorted = TopologicalSort();

        foreach (var feature in sorted)
        {
            feature.Initialize(context);
            _initialized.Add(feature.FeatureId);

            context.Events.Emit(new EditorEvent(
                EditorEventType.ModuleInstalled,
                feature.FeatureId));
        }
    }

    /// <summary>逆序关闭所有已初始化的 Feature。</summary>
    public void ShutdownAll(IEditorContext context)
    {
        for (int i = _features.Count - 1; i >= 0; i--)
        {
            var feature = _features[i];
            if (_initialized.Remove(feature.FeatureId))
            {
                try
                {
                    feature.Shutdown(context);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(
                        $"[ModuleLifecycleManager] Feature '{feature.FeatureId}' shutdown failed: {ex}");
                }
            }
        }
    }

    /// <summary>Kahn 拓扑排序。</summary>
    private List<IEditorFeature> TopologicalSort()
    {
        // 构建邻接表
        var inDegree = new Dictionary<string, int>(StringComparer.Ordinal);
        var dependents = new Dictionary<string, List<string>>(StringComparer.Ordinal);

        foreach (var f in _features)
        {
            inDegree.TryAdd(f.FeatureId, 0);
            dependents.TryAdd(f.FeatureId, new List<string>());
        }

        foreach (var f in _features)
        {
            foreach (var dep in f.Dependencies)
            {
                if (!_featureMap.ContainsKey(dep))
                {
                    throw new InvalidOperationException(
                        $"Feature '{f.FeatureId}' depends on '{dep}', which is not registered.");
                }

                inDegree[f.FeatureId]++;
                dependents[dep].Add(f.FeatureId);
            }
        }

        // BFS
        var queue = new Queue<string>();
        foreach (var (id, degree) in inDegree)
        {
            if (degree == 0)
                queue.Enqueue(id);
        }

        var sorted = new List<IEditorFeature>();

        while (queue.Count > 0)
        {
            var id = queue.Dequeue();
            sorted.Add(_featureMap[id]);

            foreach (var dependent in dependents[id])
            {
                inDegree[dependent]--;
                if (inDegree[dependent] == 0)
                    queue.Enqueue(dependent);
            }
        }

        if (sorted.Count != _features.Count)
        {
            var remaining = _features
                .Where(f => !sorted.Contains(f))
                .Select(f => f.FeatureId);
            throw new InvalidOperationException(
                $"Circular dependency detected among: {string.Join(", ", remaining)}");
        }

        return sorted;
    }
}
