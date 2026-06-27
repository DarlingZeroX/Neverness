using RmlUiNet;

namespace Neverness.Runtime.Rmlui;
/// <summary>
/// 热重载管理器。
///
/// 监听文件变化，自动重载对应的 RmlUI 文档。
/// </summary>
public sealed class HotReloader
{
    private readonly RmlDocumentManager _manager;

    /// <summary>文件路径到 Entity 的映射。</summary>
    private readonly Dictionary<string, List<ulong>> _fileToEntities = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>Entity 到文件监听路径的映射。</summary>
    private readonly Dictionary<ulong, HashSet<string>> _entityToFiles = new();

    /// <summary>文件变化回调。</summary>
    public event Action<string>? OnFileChanged;

    internal HotReloader(RmlDocumentManager manager)
    {
        _manager = manager;
    }

    /// <summary>
    /// 注册 Entity 到热重载。
    /// </summary>
    /// <param name="entity">Entity ID。</param>
    /// <param name="watchedFiles">要监听的文件路径列表。</param>
    public void RegisterEntity(ulong entity, IEnumerable<string>? watchedFiles = null)
    {
        var docPath = _manager.GetDocumentPath(entity);
        if (docPath == null) return;

        var files = watchedFiles ?? [docPath];

        if (!_entityToFiles.ContainsKey(entity))
        {
            _entityToFiles[entity] = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        }

        foreach (var file in files)
        {
            _entityToFiles[entity].Add(file);

            if (!_fileToEntities.ContainsKey(file))
            {
                _fileToEntities[file] = new List<ulong>();
            }

            if (!_fileToEntities[file].Contains(entity))
            {
                _fileToEntities[file].Add(entity);
            }
        }
    }

    /// <summary>
    /// 注销 Entity 的热重载监听。
    /// </summary>
    public void UnregisterEntity(ulong entity)
    {
        if (_entityToFiles.TryGetValue(entity, out var files))
        {
            foreach (var file in files)
            {
                if (_fileToEntities.TryGetValue(file, out var entities))
                {
                    entities.Remove(entity);
                    if (entities.Count == 0)
                    {
                        _fileToEntities.Remove(file);
                    }
                }
            }

            _entityToFiles.Remove(entity);
        }
    }

    /// <summary>
    /// 文件变化通知。
    /// </summary>
    public void NotifyFileChanged(string filePath)
    {
        OnFileChanged?.Invoke(filePath);

        if (_fileToEntities.TryGetValue(filePath, out var entities))
        {
            foreach (var entity in entities)
            {
                var doc = _manager.GetElementDocument(entity);
                if (doc != null)
                {
                    var vfsPath = _manager.GetDocumentPath(entity);
                    if (vfsPath != null)
                    {
                        _manager.UnloadDocument(entity);
                        _manager.LoadDocument(entity, vfsPath);
                    }
                }
            }
        }
    }
}
