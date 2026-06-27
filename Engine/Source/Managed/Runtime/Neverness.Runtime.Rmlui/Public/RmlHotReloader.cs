namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 热重载管理器。
///
/// 监听文件变化，自动重载对应的 RmlUI 文档。
/// 支持 .rml 和 .rcss 文件的热重载。
///
/// 使用方法：
///   var reloader = new RmlHotReloader(renderer);
///   reloader.Watch("/assets/ui/");
///   // 在文件变化回调中
///   reloader.OnFileChanged("/assets/ui/main.rml");
/// </summary>
public sealed class RmlHotReloader : IDisposable
{
    /// <summary>关联的渲染器。</summary>
    private readonly RmlRenderer _renderer;

    /// <summary>文件路径到文档的映射。</summary>
    private readonly Dictionary<string, List<RmlDocument>> _fileToDocuments = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>文档路径到文件监听路径的映射。</summary>
    private readonly Dictionary<string, HashSet<string>> _documentToFiles = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>文件变化回调。</summary>
    public event Action<string>? OnFileChanged;

    /// <summary>
    /// 创建热重载管理器。
    /// </summary>
    /// <param name="renderer">关联的渲染器。</param>
    public RmlHotReloader(RmlRenderer renderer)
    {
        _renderer = renderer;
    }

    #region 文件监听

    /// <summary>
    /// 注册文档到热重载。
    /// 当关联的文件变化时，文档会自动重载。
    /// </summary>
    /// <param name="document">要监听的文档。</param>
    /// <param name="watchedFiles">要监听的文件路径列表（VFS 路径）。</param>
    public void RegisterDocument(RmlDocument document, IEnumerable<string>? watchedFiles = null)
    {
        var docPath = document.Path;

        // 如果没有指定监听文件，使用文档路径
        var files = watchedFiles ?? [docPath];

        if (!_documentToFiles.ContainsKey(docPath))
        {
            _documentToFiles[docPath] = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        }

        foreach (var file in files)
        {
            _documentToFiles[docPath].Add(file);

            if (!_fileToDocuments.ContainsKey(file))
            {
                _fileToDocuments[file] = new List<RmlDocument>();
            }

            if (!_fileToDocuments[file].Contains(document))
            {
                _fileToDocuments[file].Add(document);
            }
        }
    }

    /// <summary>
    /// 注销文档的热重载监听。
    /// </summary>
    /// <param name="document">要注销的文档。</param>
    public void UnregisterDocument(RmlDocument document)
    {
        var docPath = document.Path;

        if (_documentToFiles.TryGetValue(docPath, out var files))
        {
            foreach (var file in files)
            {
                if (_fileToDocuments.TryGetValue(file, out var docs))
                {
                    docs.Remove(document);
                    if (docs.Count == 0)
                    {
                        _fileToDocuments.Remove(file);
                    }
                }
            }

            _documentToFiles.Remove(docPath);
        }
    }

    /// <summary>
    /// 文件变化通知。
    /// 当文件系统检测到文件变化时调用此方法。
    /// </summary>
    /// <param name="filePath">变化的文件路径（VFS 路径）。</param>
    public void NotifyFileChanged(string filePath)
    {
        if (_disposed) return;

        // 触发事件
        OnFileChanged?.Invoke(filePath);

        // 重载关联的文档
        if (_fileToDocuments.TryGetValue(filePath, out var documents))
        {
            foreach (var doc in documents)
            {
                if (doc.IsValid)
                {
                    doc.Reload();
                }
            }
        }
    }

    #endregion

    #region 样式表重载

    /// <summary>
    /// 重新加载所有文档的样式表。
    /// </summary>
    public void ReloadAllStyleSheets()
    {
        // TODO: 需要在 RmlUi.Net 中暴露文档遍历和 ReloadStyleSheet API
        foreach (var doc in _renderer.Documents)
        {
            if (doc.IsValid)
            {
                doc.Reload();
            }
        }
    }

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        _fileToDocuments.Clear();
        _documentToFiles.Clear();

        _disposed = true;
    }

    #endregion
}
