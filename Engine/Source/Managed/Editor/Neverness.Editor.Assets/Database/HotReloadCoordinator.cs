using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 热重载协调器。
///
/// 负责将 AssetWatcher / ImportPipeline / Runtime NNAssetManager 三层接线：
///   1. AssetWatcher 检测文件系统变化
///   2. ImportPipeline 执行 reimport（含 DependencyGraph 脏传播）
///   3. 通过 C ABI 通知 Runtime 侧 MarkForReload + ReloadMarkedAssets
///
/// 使用方法：
///   var coordinator = new HotReloadCoordinator(assetsRoot, libraryRoot);
///   coordinator.Start();
///   // ... 编辑器运行中 ...
///   coordinator.Dispose();
///
/// @not-threadsafe 应由主线程调用。AssetWatcher 事件从 Timer 回调触发，
///   內部 _pendingReloadGuids 使用 lock(_lock) 保护。
/// </summary>
public sealed class HotReloadCoordinator : IDisposable
{
    private readonly AssetWatcher _watcher;
    private readonly HashSet<GUID> _pendingReloadGuids = new();
    private readonly object _lock = new();
    private bool _disposed;

    /// <summary>创建热重载协调器。</summary>
    /// <param name="assetsRoot">Assets 源目录。</param>
    /// <param name="debounceMilliseconds">防抖延迟毫秒数。</param>
    public HotReloadCoordinator(NPath assetsRoot, int debounceMilliseconds = 200)
    {
        /* 使用 ImportPipeline 的共享 ImportStateCache，避免双重缓存 */
        _watcher = new AssetWatcher(assetsRoot, debounceMilliseconds, ImportPipeline.StateCache);

        _watcher.OnAssetChanged += HandleAssetChanged;
        _watcher.OnAssetCreated += HandleAssetCreated;
        _watcher.OnAssetDeleted += HandleAssetDeleted;
        _watcher.OnAssetRenamed += HandleAssetRenamed;
    }

    /* ======================== 公开 API ======================== */

    /// <summary>开始监听文件系统变化。</summary>
    public void Start()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);

        /* 确保 ImportPipeline 已初始化 */
        if (!ImportPipeline.IsInitialized)
        {
            Console.WriteLine("[HotReloadCoordinator] ImportPipeline 未初始化，跳过启动");
            return;
        }

        /* 注册导入器 */
        ImporterRegistry.Discover();

        _watcher.Start();
        Console.WriteLine("[HotReloadCoordinator] 热重载已启动");
    }

    /// <summary>停止监听。</summary>
    public void Stop()
    {
        _watcher.Stop();
        FlushRuntimeReloads();
        Console.WriteLine("[HotReloadCoordinator] 热重载已停止");
    }

    /// <summary>是否正在监听。</summary>
    public bool IsActive => _watcher.IsWatching;

    /* ======================== 事件处理 ======================== */

    /// <summary>文件内容变化 → Reimport + 脏传播 + 标记 Runtime reload。</summary>
    private void HandleAssetChanged(NPath sourcePath)
    {
        try
        {
            Console.WriteLine($"[HotReloadCoordinator] 资产变化: {sourcePath}");

            /* 重新导入 */
            var result = ImportPipeline.Reimport(sourcePath);
            if (result != null && result.Success)
            {
                MarkForRuntimeReload(result.AssetGuid);

                /* 脏传播：reimport 所有反向依赖 */
                PropagateDirtyAndReimport(result.AssetGuid, sourcePath);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] Reimport 失败: {sourcePath} → {ex.Message}");
        }
    }

    /// <summary>新文件创建 → Import。</summary>
    private void HandleAssetCreated(NPath sourcePath)
    {
        try
        {
            Console.WriteLine($"[HotReloadCoordinator] 新资产: {sourcePath}");

            var result = ImportPipeline.Import(sourcePath);
            if (result != null && result.Success)
            {
                MarkForRuntimeReload(result.AssetGuid);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] Import 失败: {sourcePath} → {ex.Message}");
        }
    }

    /// <summary>文件删除 → 清理 Database + Runtime unload。</summary>
    private void HandleAssetDeleted(NPath sourcePath)
    {
        try
        {
            Console.WriteLine($"[HotReloadCoordinator] 资产删除: {sourcePath}");

            /* 从 Database 移除 */
            EditorAssetDatabase.DeleteAsset(new NVirtualPath(sourcePath.FullPath));

            /* 从状态缓存移除 */
            ImportPipeline.StateCache.Remove(sourcePath);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] 删除处理失败: {sourcePath} → {ex.Message}");
        }
    }

    /// <summary>文件重命名 → 更新 Database 映射。</summary>
    private void HandleAssetRenamed(NPath oldPath, NPath newPath)
    {
        try
        {
            Console.WriteLine($"[HotReloadCoordinator] 资产重命名: {oldPath} → {newPath}");

            EditorAssetDatabase.MoveAsset(new NVirtualPath(oldPath.FullPath), new NVirtualPath(newPath.FullPath));

            /* 新路径需要重新导入 */
            HandleAssetCreated(newPath);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] 重命名处理失败: {oldPath} → {newPath} → {ex.Message}");
        }
    }

    /* ======================== 脏传播 ======================== */

    /// <summary>对变化资产执行 DependencyGraph 脏传播，reimport 所有反向依赖。</summary>
    private void PropagateDirtyAndReimport(GUID changedGuid, NPath sourcePath)
    {
        try
        {
            var dirtyAssets = ImportPipeline.DependencyGraph.GetDirtyPropagation(changedGuid);
            foreach (var dependentGuid in dirtyAssets)
            {
                if (dependentGuid == changedGuid)
                    continue;

                /* 通过 GUID 查找源路径并重新导入 */
                var depPath = ResolveSourcePath(dependentGuid);
                if (depPath != null)
                {
                    Console.WriteLine($"[HotReloadCoordinator] 脏传播重导入: {depPath}");
                    var depResult = ImportPipeline.Reimport(depPath.Value);
                    if (depResult != null && depResult.Success)
                    {
                        MarkForRuntimeReload(depResult.AssetGuid);
                    }
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] 脏传播失败: {ex.Message}");
        }
    }

    /* ======================== Runtime 通知 ======================== */

    /// <summary>标记资产需要 Runtime 重载（批量收集）。</summary>
    private void MarkForRuntimeReload(GUID guid)
    {
        if (guid.IsZero) return;

        lock (_lock)
        {
            _pendingReloadGuids.Add(guid);
        }
    }

    /// <summary>批量刷新 Runtime 重载请求（收集所有变化后统一通知）。</summary>
    private void FlushRuntimeReloads()
    {
        List<GUID> guids;
        lock (_lock)
        {
            if (_pendingReloadGuids.Count == 0)
                return;
            guids = new List<GUID>(_pendingReloadGuids);
            _pendingReloadGuids.Clear();
        }

        Console.WriteLine($"[HotReloadCoordinator] 刷新 {guids.Count} 个 Runtime 重载请求");

        foreach (var guid in guids)
        {
            AssetHandle.MarkForReload(guid);
        }

        AssetHandle.ReloadMarkedAssets();
    }

    /* ======================== 辅助 ======================== */

    /// <summary>通过 GUID 解析源资产路径。</summary>
    private static NPath? ResolveSourcePath(GUID guid)
    {
        if (EditorAssetDatabase.TryGetPath(guid, out var virtualPath) && !virtualPath.IsEmpty)
            return new NPath(virtualPath.FullPath);
        return null;
    }

    /* ======================== IDisposable ======================== */

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Stop();
        _watcher.Dispose();
    }
}
