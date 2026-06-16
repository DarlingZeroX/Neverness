using System.Text;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景管理器——管理多个场景世界，负责场景生命周期（Load / Unload / Activate）。
/// 替代旧的 SceneManager，使用 IScene 接口。
/// </summary>
public sealed class SceneManager
{
    /// <summary>已加载世界映射（名称 → IScene）。</summary>
    private readonly Dictionary<string, SceneWorld> _worlds = new(StringComparer.Ordinal);

    /// <summary>当前激活的世界。</summary>
    private SceneWorld? _activeWorld;

    /// <summary>场景加载完成事件（激活前触发）。</summary>
    public event Action<IScene>? SceneLoaded;

    /// <summary>场景激活事件。</summary>
    public event Action<IScene>? SceneActivated;

    /// <summary>场景卸载事件（Dispose 前触发）。</summary>
    public event Action<IScene>? SceneUnloaded;

    /// <summary>当前激活的世界。</summary>
    public SceneWorld? ActiveWorld => _activeWorld;

    /// <summary>已加载场景名称集合。</summary>
    public IReadOnlyCollection<string> LoadedSceneNames => _worlds.Keys;

    /// <summary>是否有激活场景。</summary>
    public bool HasActiveScene => _activeWorld != null;

    /// <summary>加载场景：创建 SceneWorld 并记录映射。</summary>
    public bool LoadScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (_worlds.ContainsKey(name))
        {
            return false;
        }

        var world = SceneWorld.Create(name);
        if (world == null) return false;

        _worlds[name] = world;

        SceneLoaded?.Invoke(world);

        // 首个加载的场景自动激活
        _activeWorld ??= world;
        if (_activeWorld == world)
            SceneActivated?.Invoke(world);

        return true;
    }

    /// <summary>从 VFS 资产路径加载并激活场景世界。</summary>
    /// <param name="name">场景名称。</param>
    /// <param name="vfsPath">VFS 虚拟路径（例如 "assets://scenes/main.json"）。</param>
    /// <returns>是否加载成功。</returns>
    public bool LoadSceneFromAsset(string name, string vfsPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        if (_worlds.ContainsKey(name))
        {
            // 已加载同名场景，直接激活
            return ActivateScene(name);
        }

        // 使用 SceneWorld.LoadFromAsset 加载场景
        var world = SceneWorld.LoadFromAsset(name, vfsPath);
        if (world == null) return false;

        _worlds[name] = world;
        SceneLoaded?.Invoke(world);

        // 首个加载的场景自动激活
        _activeWorld ??= world;
        if (_activeWorld == world)
            SceneActivated?.Invoke(world);

        return true;
    }

    /// <summary>从 VFS 资产路径加载并激活场景世界（二进制格式）。</summary>
    /// <param name="name">场景名称。</param>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    /// <returns>是否加载成功。</returns>
    public bool LoadSceneFromAssetBinary(string name, string vfsPath)
    {
        // 暂时使用 JSON 格式，后续可扩展二进制格式
        return LoadSceneFromAsset(name, vfsPath);
    }

    /// <summary>从流加载并激活场景世界。</summary>
    /// <param name="name">场景名称。</param>
    /// <param name="stream">场景数据流（JSON 格式）。</param>
    /// <returns>是否加载成功。</returns>
    public bool LoadSceneFromStream(string name, Stream stream)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentNullException.ThrowIfNull(stream);

        if (_worlds.ContainsKey(name))
        {
            // 已加载同名场景，直接激活
            return ActivateScene(name);
        }

        // 创建新场景
        var world = SceneWorld.Create(name);
        if (world == null) return false;

        try
        {
            world.Deserialize(stream, "json");
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"[SceneManager] 从流加载场景失败: {ex.Message}");
            world.Dispose();
            return false;
        }

        _worlds[name] = world;
        SceneLoaded?.Invoke(world);

        // 首个加载的场景自动激活
        _activeWorld ??= world;
        if (_activeWorld == world)
            SceneActivated?.Invoke(world);

        return true;
    }

    /// <summary>保存激活场景到 VFS 路径。</summary>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    /// <returns>操作结果。</returns>
    public NNSceneResult SaveActiveScene(string vfsPath)
    {
        if (_activeWorld == null) return NNSceneResult.Invalid;
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        return _activeWorld.Save(vfsPath);
    }

    /// <summary>保存激活场景到默认路径。</summary>
    /// <returns>操作结果。</returns>
    public NNSceneResult SaveActiveScene()
    {
        if (_activeWorld == null) return NNSceneResult.Invalid;
        if (string.IsNullOrEmpty(_activeWorld.AssetPath))
            return NNSceneResult.Invalid;

        return _activeWorld.Save(_activeWorld.AssetPath);
    }

    /// <summary>保存指定场景到 VFS 路径。</summary>
    /// <param name="name">场景名称。</param>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    /// <returns>是否保存成功。</returns>
    public bool SaveScene(string name, string vfsPath)
    {
        if (!_worlds.TryGetValue(name, out var world)) return false;
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        return world.Save(vfsPath).IsSuccess();
    }

    /// <summary>保存激活场景到 VFS 路径（二进制格式）。</summary>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    /// <returns>是否保存成功。</returns>
    public bool SaveActiveSceneBinary(string vfsPath)
    {
        if (_activeWorld == null) return false;
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        // 暂时使用 JSON 格式，后续可扩展二进制格式
        return _activeWorld.Save(vfsPath).IsSuccess();
    }

    /// <summary>卸载场景：销毁世界及其所有资源，移除映射。</summary>
    public bool UnloadScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (!_worlds.TryGetValue(name, out var world))
        {
            return false;
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

        return true;
    }

    /// <summary>激活指定名称的已加载场景。</summary>
    public bool ActivateScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        if (!_worlds.TryGetValue(name, out var world))
        {
            return false;
        }

        _activeWorld = world;
        SceneActivated?.Invoke(world);
        return true;
    }

    /// <summary>查询场景是否已加载。</summary>
    public bool IsSceneLoaded(string name) => _worlds.ContainsKey(name);

    /// <summary>获取已加载的世界。</summary>
    public SceneWorld? GetWorld(string name)
    {
        _worlds.TryGetValue(name, out var world);
        return world;
    }

    /// <summary>驱动激活场景的 ECS System Tick。</summary>
    public void TickActiveScene(float deltaTime)
    {
        _activeWorld?.Tick(deltaTime);
    }

    /// <summary>在激活场景中创建实体。</summary>
    public SceneEntity? CreateEntity(string? displayName = null)
    {
        if (_activeWorld == null) return null;
        var entity = _activeWorld.CreateEntity(displayName);
        return new SceneEntity(entity, _activeWorld);
    }

    /// <summary>在指定场景中创建实体。</summary>
    public SceneEntity? CreateEntityIn(string sceneName, string? displayName = null)
    {
        if (!_worlds.TryGetValue(sceneName, out var world)) return null;
        var entity = world.CreateEntity(displayName);
        return new SceneEntity(entity, world);
    }

    /// <summary>销毁实体。</summary>
    public bool DestroyEntity(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        if (!entity.IsAlive) return false;

        entity.Entity.Destroy();
        return true;
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

    /// <summary>获取已加载场景的数量。</summary>
    public int LoadedCount => _worlds.Count;
}
