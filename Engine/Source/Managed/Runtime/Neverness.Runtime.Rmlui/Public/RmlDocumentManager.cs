using RmlUiNet;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 文档管理器。
///
/// 使用 Entity（uint64_t）管理文档，对应 C++ RmlUIRenderer::Sync() 的逻辑。
/// 包含热重载功能。
///
/// 使用方法：
///   var docMgr = new RmlDocumentManager(context);
///   docMgr.Sync([(entity1, "/ui/main.rml"), (entity2, "/ui/hud.rml")]);
///   docMgr.NotifyFileChanged("/ui/main.rml"); // 触发热重载
/// </summary>
public sealed class RmlDocumentManager
{
    /// <summary>RmlUi Context。</summary>
    private readonly Context _context;

    /// <summary>Entity → 文档运行时实例。</summary>
    private readonly Dictionary<ulong, DocRuntime> _documents = new();

    /// <summary>热重载管理器。</summary>
    private readonly HotReloader _hotReloader;

    /// <summary>
    /// 创建文档管理器。
    /// </summary>
    /// <param name="context">RmlUi Context。</param>
    public RmlDocumentManager(Context context)
    {
        _context = context;
        _hotReloader = new HotReloader(this);
    }

    #region 内部类型

    /// <summary>
    /// 文档运行时实例。
    /// </summary>
    private class DocRuntime
    {
        public ElementDocument? Document;
        public string VfsPath = string.Empty;
        public ulong Entity;
        public DocState State;
    }

    /// <summary>
    /// 文档状态。
    /// </summary>
    private enum DocState : byte
    {
        Ready,      ///< 已加载，可渲染
        Hidden,     ///< 已加载但不可见
        Failed,     ///< 加载失败
    }

    #endregion

    #region 属性

    /// <summary>
    /// 当前活跃的 Entity 集合。
    /// </summary>
    public IReadOnlyCollection<ulong> ActiveEntities => _documents.Keys;

    /// <summary>
    /// 活跃文档数量。
    /// </summary>
    public int Count => _documents.Count;

    /// <summary>
    /// 热重载管理器。
    /// </summary>
    public HotReloader HotReload => _hotReloader;

    #endregion

    #region 同步（三路 Diff）

    /// <summary>
    /// 同步文档列表（三路 Diff）。
    ///
    /// 对应 C++ RmlUIRenderer::Sync()：
    /// 1. 删除不在新列表中的文档
    /// 2. 加载新列表中的文档
    /// 3. 保持已存在的文档
    /// </summary>
    /// <param name="items">要同步的 (Entity, VFS路径) 列表。</param>
    public void Sync(IEnumerable<(ulong Entity, string VfsPath)> items)
    {
        var newItems = new Dictionary<ulong, string>();
        foreach (var item in items)
        {
            newItems[item.Entity] = item.VfsPath;
        }

        // 1. 删除不在新列表中的文档
        var toRemove = new List<ulong>();
        foreach (var entity in _documents.Keys)
        {
            if (!newItems.ContainsKey(entity))
            {
                toRemove.Add(entity);
            }
        }

        foreach (var entity in toRemove)
        {
            UnloadDocument(entity);
        }

        // 2. 加载新文档
        foreach (var (entity, vfsPath) in newItems)
        {
            if (_documents.ContainsKey(entity))
                continue; // 已存在，跳过

            LoadDocument(entity, vfsPath);
        }
    }

    #endregion

    #region 单文档操作

    /// <summary>
    /// 加载文档。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    /// <param name="vfsPath">文档 VFS 路径。</param>
    /// <returns>是否成功加载。</returns>
    public bool LoadDocument(ulong entity, string vfsPath)
    {
        // 检查是否已存在
        if (_documents.ContainsKey(entity))
            return true;

        // 加载文档
        var doc = _context.LoadDocument(vfsPath);

        var runtime = new DocRuntime
        {
            Document = doc,
            VfsPath = vfsPath,
            Entity = entity,
            State = doc != null ? DocState.Ready : DocState.Failed,
        };

        _documents[entity] = runtime;

        if (doc != null)
        {
            doc.Show();
            Console.WriteLine($"[RmlDocumentManager] Loaded: entity={entity}, path={vfsPath}");
        }
        else
        {
            Console.Error.WriteLine($"[RmlDocumentManager] Failed to load: entity={entity}, path={vfsPath}");
        }

        return doc != null;
    }

    /// <summary>
    /// 卸载文档。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    public void UnloadDocument(ulong entity)
    {
        if (!_documents.TryGetValue(entity, out var runtime))
            return;

        runtime.Document?.Close();
        _documents.Remove(entity);

        // 注销热重载监听
        _hotReloader.UnregisterEntity(entity);

        Console.WriteLine($"[RmlDocumentManager] Unloaded: entity={entity}");
    }

    /// <summary>
    /// 获取底层 ElementDocument。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    /// <returns>ElementDocument 实例，未找到返回 null。</returns>
    public ElementDocument? GetElementDocument(ulong entity)
    {
        if (_documents.TryGetValue(entity, out var runtime))
            return runtime.Document;

        return null;
    }

    /// <summary>
    /// 获取文档路径。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    /// <returns>文档路径，未找到返回 null。</returns>
    public string? GetDocumentPath(ulong entity)
    {
        if (_documents.TryGetValue(entity, out var runtime))
            return runtime.VfsPath;

        return null;
    }

    /// <summary>
    /// 检查 Entity 是否存在。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    /// <returns>是否存在。</returns>
    public bool Contains(ulong entity)
    {
        return _documents.ContainsKey(entity);
    }

    #endregion

    #region 文档操作

    /// <summary>
    /// 显示文档。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    public void ShowDocument(ulong entity)
    {
        if (_documents.TryGetValue(entity, out var runtime))
        {
            runtime.Document?.Show();
            runtime.State = DocState.Ready;
        }
    }

    /// <summary>
    /// 隐藏文档。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    public void HideDocument(ulong entity)
    {
        if (_documents.TryGetValue(entity, out var runtime))
        {
            runtime.Document?.Hide();
            runtime.State = DocState.Hidden;
        }
    }

    #endregion

    #region 批量操作

    /// <summary>
    /// 清除所有文档。
    /// </summary>
    public void Clear()
    {
        foreach (var (entity, runtime) in _documents)
        {
            runtime.Document?.Close();
        }
        _documents.Clear();
    }

    /// <summary>
    /// 重新加载所有文档。
    /// </summary>
    public void ReloadAll()
    {
        foreach (var (entity, runtime) in _documents)
        {
            if (runtime.Document != null)
            {
                var vfsPath = runtime.VfsPath;
                var visible = runtime.State == DocState.Ready;

                // 关闭旧文档
                runtime.Document.Close();

                // 重新加载
                runtime.Document = _context.LoadDocument(vfsPath);
                if (runtime.Document != null)
                {
                    if (visible)
                        runtime.Document.Show();
                    runtime.State = DocState.Ready;
                }
                else
                {
                    runtime.State = DocState.Failed;
                }
            }
        }
    }

    /// <summary>
    /// 显示所有文档。
    /// </summary>
    public void ShowAll()
    {
        foreach (var (entity, runtime) in _documents)
        {
            runtime.Document?.Show();
            runtime.State = DocState.Ready;
        }
    }

    /// <summary>
    /// 隐藏所有文档。
    /// </summary>
    public void HideAll()
    {
        foreach (var (entity, runtime) in _documents)
        {
            runtime.Document?.Hide();
            runtime.State = DocState.Hidden;
        }
    }

    #endregion

    #region 热重载委托

    /// <summary>
    /// 文件变化通知（委托给 HotReloader）。
    /// </summary>
    public void NotifyFileChanged(string filePath)
    {
        _hotReloader.NotifyFileChanged(filePath);
    }

    /// <summary>
    /// 注册 Entity 到热重载。
    /// </summary>
    public void RegisterForReload(ulong entity, IEnumerable<string>? watchedFiles = null)
    {
        _hotReloader.RegisterEntity(entity, watchedFiles);
    }

    /// <summary>
    /// 注销 Entity 的热重载监听。
    /// </summary>
    public void UnregisterForReload(ulong entity)
    {
        _hotReloader.UnregisterEntity(entity);
    }

    #endregion
}
