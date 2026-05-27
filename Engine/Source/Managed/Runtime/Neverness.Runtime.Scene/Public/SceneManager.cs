using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景管理器——降级为"世界管理器"，负责场景生命周期（Load / Unload / Activate）。
/// 实体管理委托给 <see cref="SceneWorld.Entities"/>，不再直接持有实体列表。
/// </summary>
public sealed class SceneManager
{
    /// <summary>已加载世界映射（名称 → SceneWorld）。</summary>
    private readonly Dictionary<string, SceneWorld> _worlds = new(StringComparer.Ordinal);

    /// <summary>当前激活的世界。</summary>
    private SceneWorld? _activeWorld;

    /// <summary>场景加载完成事件（激活前触发）。</summary>
    public event Action<SceneWorld>? SceneLoaded;

        /// <summary>场景激活事件。</summary>
    public event Action<SceneWorld>? SceneActivated;

        /// <summary>场景卸载事件（Dispose 前触发）。</summary>
    public event Action<SceneWorld>? SceneUnloaded;

        /// <summary>当前激活的世界。</summary>
    public SceneWorld? ActiveWorld => _activeWorld;

    /// <summary>当前激活场景的 Native 句柄（兼容旧 API）。</summary>
    public ulong ActiveScene => _activeWorld?.NativeHandle ?? 0;

    /// <summary>已加载场景名称集合。</summary>
    public IReadOnlyCollection<string> LoadedSceneNames => _worlds.Keys;

    /// <summary>本会话跟踪的实体（兼容旧 API，委托激活世界）。</summary>
    public IReadOnlyList<SceneEntity> Entities => _activeWorld?.Entities.Entities ?? [];

    /// <summary>是否有激活场景。</summary>
    public bool HasActiveScene => _activeWorld != null;

    /// <summary>加载场景：创建 <see cref="SceneWorld"/> 并记录映射。</summary>
    public NNSceneResult LoadScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (_worlds.ContainsKey(name))
        {
            return NNSceneResult.Invalid;
        }

        var world = SceneWorld.Create(name);
        if (world == null)
        {
            return NNSceneResult.Invalid;
        }

        _worlds[name] = world;

        SceneLoaded?.Invoke(world);

        // 首个加载的场景自动激活
        _activeWorld ??= world;
        if (_activeWorld == world)
            SceneActivated?.Invoke(world);

        return NNSceneResult.Ok;
    }

    /// <summary>卸载场景：销毁世界及其所有资源，移除映射。</summary>
    public NNSceneResult UnloadScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (!_worlds.TryGetValue(name, out var world))
        {
            return NNSceneResult.NotFound;
        }

        var isActive = _activeWorld == world;

        SceneUnloaded?.Invoke(world);
        world.Dispose();
        _worlds.Remove(name);

        // 若卸载的是激活场景，切换到第一个可用世界或清零
        if (isActive)
        {
            _activeWorld = _worlds.Count > 0
                ? _worlds.Values.First()
                : null;
        }

        return NNSceneResult.Ok;
    }

    /// <summary>激活指定名称的已加载场景。</summary>
    public NNSceneResult ActivateScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (!_worlds.TryGetValue(name, out var world))
        {
            return NNSceneResult.NotFound;
        }

        _activeWorld = world;
        SceneActivated?.Invoke(world);
            return NNSceneResult.Ok;
    }

        /// <summary>从 VFS 资产路径加载并激活场景世界。</summary>
    public NNSceneResult LoadSceneFromAsset(string name, string vfsPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (_worlds.ContainsKey(name))
        {
            // 已加载同名场景，直接激活
            return ActivateScene(name);
        }

        var world = SceneWorld.LoadFromAsset(name, vfsPath);
        if (world == null)
        {
            return NNSceneResult.Invalid;
        }

        _worlds[name] = world;
        SceneLoaded?.Invoke(world);

        // 首个加载的场景自动激活
        _activeWorld ??= world;
        if (_activeWorld == world)
            SceneActivated?.Invoke(world);

        return NNSceneResult.Ok;
    }

    /// <summary>查询场景是否已加载。</summary>
    public bool IsSceneLoaded(string name) => _worlds.ContainsKey(name);

    /// <summary>获取已加载场景的 Native 句柄（未加载返回 0）。</summary>
    public ulong GetSceneHandle(string name)
    {
        return _worlds.TryGetValue(name, out var world) ? world.NativeHandle : 0;
    }

    /// <summary>获取已加载的世界。</summary>
    public SceneWorld? GetWorld(string name)
    {
        _worlds.TryGetValue(name, out var world);
        return world;
    }

    /// <summary>驱动激活场景的 ECS System Tick（委托激活世界的 Tick）。</summary>
    public void TickActiveScene(float deltaTime)
    {
        _activeWorld?.Tick(deltaTime);
    }

    /// <summary>保存激活场景到其资产路径。无路径或无激活场景时返回 Invalid。</summary>
    public NNSceneResult SaveActiveScene()
    {
        if (_activeWorld == null)
            return NNSceneResult.Invalid;
        if (string.IsNullOrEmpty(_activeWorld.AssetPath))
            return NNSceneResult.Invalid;
        return _activeWorld.Save(_activeWorld.AssetPath);
    }

    /// <summary>保存激活场景到指定路径，并更新 AssetPath。</summary>
    public NNSceneResult SaveActiveSceneAs(string vfsPath)
    {
        if (_activeWorld == null)
            return NNSceneResult.Invalid;
        var result = _activeWorld.Save(vfsPath);
        if (result == NNSceneResult.Ok)
            _activeWorld.AssetPath = vfsPath;
        return result;
    }

    /// <summary>在激活场景中创建实体（兼容旧 API，委托激活世界）。</summary>
    public SceneEntity? CreateEntity(string? displayName = null)
    {
        return _activeWorld?.CreateEntity(displayName);
    }

    /// <summary>在指定场景中创建实体。</summary>
    public SceneEntity? CreateEntityIn(string sceneName, string? displayName = null)
    {
        return _worlds.TryGetValue(sceneName, out var world)
            ? world.CreateEntity(displayName)
            : null;
    }

    /// <summary>销毁实体（兼容旧 API，委托对应世界的注册表）。</summary>
    public NNSceneResult DestroyEntity(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);

        // 查找实体所属世界
        foreach (var world in _worlds.Values)
        {
            if (world.Entities.Contains(entity))
            {
                return world.DestroyEntity(entity);
            }
        }

        return NNSceneResult.NotFound;
    }

    /// <summary>将已有实体加入跟踪（兼容旧 API，委托激活世界）。</summary>
    public void TrackEntity(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        _activeWorld?.Entities.Register(entity);
    }

    /// <summary>清理所有世界（用于应用退出或重置）。</summary>
    public void Clear()
    {
        foreach (var world in _worlds.Values)
        {
            world.Dispose();
        }

        _worlds.Clear();
        _activeWorld = null;
    }

    // ── 热重载 ──

    /// <summary>
    /// 保存所有已加载世界的热重载快照。
    /// 在程序集卸载前调用，捕获当前运行时状态。
    /// </summary>
    public GlobalHotReloadSnapshot SaveAllSnapshots()
    {
        var snapshots = new Dictionary<string, HotReloadSnapshot>(_worlds.Count);

        foreach (var (name, world) in _worlds)
        {
            snapshots[name] = world.SaveSnapshot();
        }

        // 找出激活世界名称
        string? activeName = null;
        if (_activeWorld != null)
        {
            foreach (var (name, world) in _worlds)
            {
                if (world == _activeWorld)
                {
                    activeName = name;
                    break;
                }
            }
        }

        return new GlobalHotReloadSnapshot
        {
            Worlds = snapshots,
            ActiveWorldName = activeName,
            CapturedAt = DateTimeOffset.UtcNow,
        };
    }

    /// <summary>
    /// 从全局热重载快照恢复所有世界。
    /// 在新程序集加载后调用，重建 Managed 侧运行时状态。
    /// </summary>
    public void RestoreAllSnapshots(GlobalHotReloadSnapshot snapshot)
    {
        ArgumentNullException.ThrowIfNull(snapshot);

        // 先清理当前所有世界（不销毁 Native 场景，因为快照中的句柄仍然有效）
        foreach (var world in _worlds.Values)
        {
            // 仅释放 Managed 资源，不销毁 Native 场景
            world.Systems.Dispose();
            world.Events.Clear();
            world.Queries.Clear();
            world.Entities.Clear();
        }

        _worlds.Clear();
        _activeWorld = null;

        // 从快照恢复每个世界
        foreach (var (name, worldSnapshot) in snapshot.Worlds)
        {
            var world = SceneWorld.RestoreFromSnapshot(worldSnapshot);
            if (world != null)
            {
                _worlds[name] = world;
            }
        }

        // 恢复激活世界
        if (snapshot.ActiveWorldName != null &&
            _worlds.TryGetValue(snapshot.ActiveWorldName, out var activeWorld))
        {
            _activeWorld = activeWorld;
        }
        else if (_worlds.Count > 0)
        {
            _activeWorld = _worlds.Values.First();
        }
    }

    /// <summary>
    /// 热重载后重建所有世界的 Managed 侧状态。
    /// 在 <see cref="RestoreAllSnapshots"/> 之后、重新注册 System 之前调用。
    /// </summary>
    public void RebuildAllAfterReload()
    {
        foreach (var world in _worlds.Values)
        {
            world.RebuildAfterReload();
        }
    }
}
